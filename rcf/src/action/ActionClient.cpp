#include "rcf/action/ActionClient.hpp"
#include "rcf/Serializer.hpp"
#include "rcf/Logger.hpp"

#include <stdexcept>
#include <thread>
#include <cstring>

namespace rcf {

// ─────────────────────────────────────────────────────────────────────────────
//  ClientGoalHandle<Feedback, Result>
// ─────────────────────────────────────────────────────────────────────────────

template<typename Feedback, typename Result>
ClientGoalHandle<Feedback, Result>::ClientGoalHandle() {
    status_atomic  = std::make_shared<std::atomic<GoalStatus>>(GoalStatus::IDLE);
    result_promise = std::make_shared<std::promise<std::pair<GoalStatus, Result>>>();
    result_future  = result_promise->get_future().share();
}

template<typename Feedback, typename Result>
GoalStatus ClientGoalHandle<Feedback, Result>::getStatus() const {
    return status_atomic->load(std::memory_order_acquire);
}

template<typename Feedback, typename Result>
std::pair<GoalStatus, Result>
ClientGoalHandle<Feedback, Result>::waitForResult(int timeout_ms) {
    if (timeout_ms < 0)
        return result_future.get();

    if (result_future.wait_for(std::chrono::milliseconds(timeout_ms))
            == std::future_status::timeout)
        throw std::runtime_error("waitForResult timeout");

    return result_future.get();
}

// ─────────────────────────────────────────────────────────────────────────────
//  ActionClient<Goal, Feedback, Result>
// ─────────────────────────────────────────────────────────────────────────────

template<typename Goal, typename Feedback, typename Result>
ActionClient<Goal, Feedback, Result>::ActionClient(
    const std::string& base, int timeout_ms)
    : mode_(Mode::UNIX), base_(base), timeout_ms_(timeout_ms)
{}

template<typename Goal, typename Feedback, typename Result>
ActionClient<Goal, Feedback, Result>::ActionClient(
    const std::string& host, uint16_t port_base, int timeout_ms)
    : mode_(Mode::TCP), host_(host), port_base_(port_base), timeout_ms_(timeout_ms)
{}

// ── connect helper ────────────────────────────────────────────────────────────
template<typename Goal, typename Feedback, typename Result>
std::unique_ptr<ITransport>
ActionClient<Goal, Feedback, Result>::connect(int offset) const {
    static constexpr const char* suffixes[] = { "_goal", "_cancel", "_result" };

    if (mode_ == Mode::UNIX)
        return std::make_unique<UnixTransport>(base_ + suffixes[offset], timeout_ms_);
    else
        return std::make_unique<TcpTransport>(
            host_, uint16_t(port_base_ + offset), timeout_ms_);
}

// ── sendGoal ─────────────────────────────────────────────────────────────────
template<typename Goal, typename Feedback, typename Result>
typename ActionClient<Goal, Feedback, Result>::CGHPtr
ActionClient<Goal, Feedback, Result>::sendGoal(
    const Goal& goal, FeedbackCb fb_cb, StatusCb st_cb)
{
    auto cgh = std::make_shared<CGH>();

    // ── Step 1: open goal channel, send goal request, receive ACK ─────────────
    // The connection is short-lived (request-response), then closed.
    {
        auto t = connect(0);

        // Wire format: [GOAL_REQUEST byte (1)] [Goal struct (sizeof(Goal))]
        std::vector<uint8_t> payload;
        payload.push_back(uint8_t(ActionMsgType::GOAL_REQUEST));
        const uint8_t* gp = reinterpret_cast<const uint8_t*>(&goal);
        payload.insert(payload.end(), gp, gp + sizeof(Goal));
        t->sendFrame(Serializer::encodeVec(payload));

        GoalAckMsg ack = Serializer::decodeStruct<GoalAckMsg>(
            t->recvFrame(timeout_ms_));
        cgh->goal_id = ack.goal_id;
        cgh->status_atomic->store(ack.status, std::memory_order_release);

        // Server rejected the goal — resolve future immediately, return early
        if (ack.status == GoalStatus::REJECTED) {
            RIPC_WARN("ActionClient", ack.goal_id.str() + " REJECTED");
            try {
                cgh->result_promise->set_value({ GoalStatus::REJECTED, Result{} });
            } catch (...) {}
            return cgh;
        }
    }
    RIPC_INFO("ActionClient", cgh->goal_id.str() + " ACCEPTED");
    cgh->status_atomic->store(GoalStatus::ACCEPTED, std::memory_order_release);

    // Capture locals for background threads (no this-pointer capture —
    // caller may destroy ActionClient before the threads finish)
    GoalID  gid        = cgh->goal_id;
    auto    sa         = cgh->status_atomic;
    auto    promise    = cgh->result_promise;
    auto    self_mode  = mode_;
    auto    self_base  = base_;
    auto    self_host  = host_;
    auto    self_port  = port_base_;
    auto    self_tms   = timeout_ms_;

    // ── Step 2: result-waiter thread ─────────────────────────────────────────
    // Opens the result channel and blocks until the server sends the result
    // (which only happens after setSucceeded/setCanceled/setAborted).
    std::thread([=]() mutable {
        try {
            std::unique_ptr<ITransport> rt;
            if (self_mode == Mode::UNIX)
                rt = std::make_unique<UnixTransport>(self_base + "_result", self_tms);
            else
                rt = std::make_unique<TcpTransport>(
                    self_host, uint16_t(self_port + 2), self_tms);

            // Request the result for our specific goal ID
            ResultRequestMsg req{};
            req.type    = ActionMsgType::RESULT_REQUEST;
            req.goal_id = gid;
            rt->sendFrame(Serializer::encodeStruct(req));

            // Server blocks until terminal state, then sends:
            //   [RESULT_RESPONSE byte][status byte][Result bytes]
            auto rp = rt->recvFrame(60000);   // 60s max wait
            if (rp.size() < 2 + sizeof(Result))
                throw std::runtime_error("Result payload too short");

            GoalStatus fs = static_cast<GoalStatus>(rp[1]);
            Result r{};
            std::memcpy(&r, rp.data() + 2, sizeof(Result));

            sa->store(fs, std::memory_order_release);
            if (st_cb) st_cb(gid, fs);
            RIPC_INFO("ActionClient", gid.str() + " final=" + goalStatusStr(fs));
            try {
                promise->set_value({ fs, r });
            } catch (...) {}

        } catch (const std::exception& e) {
            RIPC_ERROR("ActionClient::result", e.what());
            sa->store(GoalStatus::ABORTED, std::memory_order_release);
            if (st_cb) st_cb(gid, GoalStatus::ABORTED);
            try {
                promise->set_value({ GoalStatus::ABORTED, Result{} });
            } catch (...) {}
        }
    }).detach();

    // ── Step 3: feedback-polling thread (only if caller wants feedback) ───────
    // Feedback is pushed by the server via publishFeedback(); because we
    // use a stateless per-call socket model, the client side polls the
    // result socket for embedded feedback packets.
    // NOTE: the current ActionServer pushes feedback only through the GoalHandle
    // callback (set via setFeedbackCallback). For a full feedback channel,
    // add a fourth socket.  Here we provide a no-op stub that keeps the API
    // consistent — the feedback callback can still be wired at the server
    // level if using the in-process GoalHandle directly.
    if (fb_cb) {
        // Stub: feedback over a dedicated fourth socket would go here.
        // For the current Unix/TCP layout (3 sockets), feedback is delivered
        // only when the client and server share the GoalHandle directly
        // (same-process or via the MmapStream layer).
        RIPC_DEBUG("ActionClient",
                   gid.str() + " fb_cb registered (stub — wire feedback TBD)");
    }

    return cgh;
}

// ── cancel ────────────────────────────────────────────────────────────────────
template<typename Goal, typename Feedback, typename Result>
bool ActionClient<Goal, Feedback, Result>::cancel(CGHPtr cgh) {
    if (!cgh) return false;
    try {
        auto t = connect(1);  // cancel channel
        CancelRequestMsg req{};
        req.type    = ActionMsgType::CANCEL_REQUEST;
        req.goal_id = cgh->goal_id;
        t->sendFrame(Serializer::encodeStruct(req));

        CancelAckMsg ack = Serializer::decodeStruct<CancelAckMsg>(
            t->recvFrame(timeout_ms_));

        RIPC_INFO("ActionClient",
                  cgh->goal_id.str() +
                  (ack.ok ? " cancel ACK" : " cancel: goal not found"));
        return ack.ok != 0;

    } catch (const std::exception& e) {
        RIPC_ERROR("ActionClient::cancel", e.what());
        return false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Explicit instantiations
//
//  Add your <Goal, Feedback, Result> triples below.
//  Mirror these in ActionServer.cpp and GoalHandle.cpp.
//
//  Pattern:
//    #include "MyTypes.hpp"
//    namespace rcf {
//      template struct ClientGoalHandle<MyFeedback, MyResult>;
//      template class  ActionClient<MyGoal, MyFeedback, MyResult>;
//    }
// ─────────────────────────────────────────────────────────────────────────────

} // namespace rcf
