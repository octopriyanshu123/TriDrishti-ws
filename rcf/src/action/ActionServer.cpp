#include "rcf/action/ActionServer.hpp"
#include "rcf/Serializer.hpp"
#include "rcf/Logger.hpp"

#include <cstring>
#include <stdexcept>

namespace rcf {

// ─────────────────────────────────────────────────────────────────────────────
//  ActionServer<Goal, Feedback, Result>  —  full method bodies
// ─────────────────────────────────────────────────────────────────────────────

template<typename Goal, typename Feedback, typename Result>
ActionServer<Goal, Feedback, Result>::~ActionServer() { shutdown(); }

// ── Factories ─────────────────────────────────────────────────────────────────
template<typename Goal, typename Feedback, typename Result>
std::unique_ptr<ActionServer<Goal, Feedback, Result>>
ActionServer<Goal, Feedback, Result>::makeUnix(const std::string& base) {
    auto s   = std::make_unique<ActionServer>();
    s->mode_ = Mode::UNIX;
    s->base_ = base;
    return s;
}

template<typename Goal, typename Feedback, typename Result>
std::unique_ptr<ActionServer<Goal, Feedback, Result>>
ActionServer<Goal, Feedback, Result>::makeTcp(uint16_t port_base) {
    auto s        = std::make_unique<ActionServer>();
    s->mode_      = Mode::TCP;
    s->port_base_ = port_base;
    return s;
}

// ── Setup ─────────────────────────────────────────────────────────────────────
template<typename Goal, typename Feedback, typename Result>
void ActionServer<Goal, Feedback, Result>::onExecute(ExecFn f) {
    exec_fn_ = std::move(f);
}

template<typename Goal, typename Feedback, typename Result>
void ActionServer<Goal, Feedback, Result>::onGoalCheck(CheckFn f) {
    check_fn_ = std::move(f);
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────
template<typename Goal, typename Feedback, typename Result>
void ActionServer<Goal, Feedback, Result>::spin() {
    if (!exec_fn_) throw std::runtime_error("ActionServer: no execute callback");
    startServers();
    running_.store(true);
    RIPC_INFO("ActionServer", "Ready");

    // cancel and result channels run in their own threads
    std::thread ct([this] {
        acceptLoop(cancel_srv_.get(),
                   [this](auto t) { handleCancel(std::move(t)); });
    });
    std::thread rt([this] {
        acceptLoop(result_srv_.get(),
                   [this](auto t) { handleResult(std::move(t)); });
    });

    // goal channel blocks this thread
    acceptLoop(goal_srv_.get(),
               [this](auto t) { handleGoal(std::move(t)); });

    ct.join();
    rt.join();
}

template<typename Goal, typename Feedback, typename Result>
void ActionServer<Goal, Feedback, Result>::spinAsync() {
    if (!exec_fn_)
        throw std::runtime_error(
            "ActionServer::spinAsync() called without onExecute() -- "
            "call onExecute(fn) before spinAsync()");
    spin_thread_ = std::thread([this] { spin(); });
}

template<typename Goal, typename Feedback, typename Result>
void ActionServer<Goal, Feedback, Result>::shutdown() {
    running_.store(false);
    if (goal_srv_)   goal_srv_->close();
    if (cancel_srv_) cancel_srv_->close();
    if (result_srv_) result_srv_->close();
    if (spin_thread_.joinable()) spin_thread_.join();
}

// ── Private helpers ───────────────────────────────────────────────────────────
template<typename Goal, typename Feedback, typename Result>
void ActionServer<Goal, Feedback, Result>::startServers() {
    if (mode_ == Mode::UNIX) {
        goal_srv_   = std::make_unique<UnixTransportServer>(base_ + "_goal");
        cancel_srv_ = std::make_unique<UnixTransportServer>(base_ + "_cancel");
        result_srv_ = std::make_unique<UnixTransportServer>(base_ + "_result");
    } else {
        goal_srv_   = std::make_unique<TcpTransportServer>(port_base_);
        cancel_srv_ = std::make_unique<TcpTransportServer>(port_base_ + 1);
        result_srv_ = std::make_unique<TcpTransportServer>(port_base_ + 2);
    }
    goal_srv_->listen();
    cancel_srv_->listen();
    result_srv_->listen();
}

template<typename Goal, typename Feedback, typename Result>
void ActionServer<Goal, Feedback, Result>::acceptLoop(
    ITransportServer* srv,
    std::function<void(std::unique_ptr<ITransport>)> handler)
{
    while (running_.load()) {
        try {
            auto t = srv->accept();
            if (!running_.load()) break;
            std::thread([h = std::move(handler), t2 = std::move(t)]() mutable {
                h(std::move(t2));
            }).detach();
        } catch (const std::exception& e) {
            if (running_.load()) RIPC_ERROR("ActionServer::accept", e.what());
        }
    }
}

template<typename Goal, typename Feedback, typename Result>
void ActionServer<Goal, Feedback, Result>::handleGoal(
    std::unique_ptr<ITransport> t)
{
    try {
        auto payload = t->recvFrame(5000);
        if (payload.empty()) return;

        // First byte is ActionMsgType
        if (static_cast<ActionMsgType>(payload[0]) != ActionMsgType::GOAL_REQUEST)
            return;
        if (payload.size() < 1 + sizeof(Goal)) {
            RIPC_ERROR("ActionServer", "Goal payload too short");
            return;
        }

        Goal g;
        std::memcpy(&g, payload.data() + 1, sizeof(Goal));

        GoalID id{ counter_.fetch_add(1) };
        bool   ok = !check_fn_ || check_fn_(g);

        GoalAckMsg ack{};
        ack.goal_id = id;

        if (!ok) {
            ack.status = GoalStatus::REJECTED;
            t->sendFrame(Serializer::encodeStruct(ack));
            RIPC_WARN("ActionServer", "Rejected " + id.str());
            return;
        }

        // Store handle before ACK so cancel/result handlers can find it
        auto handle = std::make_shared<Handle>(id);
        {
            std::lock_guard<std::mutex> lk(goals_mutex_);
            goals_[id.id] = handle;
        }

        ack.status = GoalStatus::ACCEPTED;
        t->sendFrame(Serializer::encodeStruct(ack));
        RIPC_INFO("ActionServer", "Accepted " + id.str());

        // Run execute callback in a detached worker thread
        std::thread([this, g, handle]() mutable {
            handle->transitionToExecuting();
            RIPC_INFO("ActionServer", handle->goalId().str() + " EXECUTING");
            try {
                exec_fn_(g, handle);
            } catch (const std::exception& e) {
                RIPC_ERROR("ActionServer", "Worker: " + std::string(e.what()));
                if (!handle->isTerminal())
                    handle->setAborted(Result{});
            }
            // Keep handle alive briefly so result requester can fetch it
            std::this_thread::sleep_for(std::chrono::seconds(5));
            std::lock_guard<std::mutex> lk(goals_mutex_);
            goals_.erase(handle->goalId().id);
        }).detach();

    } catch (const std::exception& e) {
        RIPC_ERROR("ActionServer::handleGoal", e.what());
    }
}

template<typename Goal, typename Feedback, typename Result>
void ActionServer<Goal, Feedback, Result>::handleCancel(
    std::unique_ptr<ITransport> t)
{
    try {
        auto payload = t->recvFrame(3000);
        CancelRequestMsg req;
        std::memcpy(&req, payload.data(), sizeof(req));

        HandlePtr h;
        {
            std::lock_guard<std::mutex> lk(goals_mutex_);
            auto it = goals_.find(req.goal_id.id);
            if (it != goals_.end()) h = it->second;
        }

        CancelAckMsg ack{};
        if (h) { h->requestCancel(); ack.ok = 1; }
        else   {                      ack.ok = 0; }
        t->sendFrame(Serializer::encodeStruct(ack));

    } catch (const std::exception& e) {
        RIPC_ERROR("ActionServer::handleCancel", e.what());
    }
}

template<typename Goal, typename Feedback, typename Result>
void ActionServer<Goal, Feedback, Result>::handleResult(
    std::unique_ptr<ITransport> t)
{
    try {
        auto payload = t->recvFrame(3000);
        ResultRequestMsg req;
        std::memcpy(&req, payload.data(), sizeof(req));

        HandlePtr h;
        {
            std::lock_guard<std::mutex> lk(goals_mutex_);
            auto it = goals_.find(req.goal_id.id);
            if (it != goals_.end()) h = it->second;
        }

        if (!h) {
            RIPC_WARN("ActionServer", "Result: goal " +
                      req.goal_id.str() + " not found");
            return;
        }

        // Block until the execute worker reaches a terminal state (max 60s)
        h->waitForResult(60000);

        // Build result response: [RESULT_RESPONSE byte][status byte][Result bytes]
        std::vector<uint8_t> rp;
        rp.push_back(uint8_t(ActionMsgType::RESULT_RESPONSE));
        rp.push_back(uint8_t(h->status()));
        Result r = h->getResult();
        const uint8_t* rptr = reinterpret_cast<const uint8_t*>(&r);
        rp.insert(rp.end(), rptr, rptr + sizeof(Result));

        t->sendFrame(Serializer::encodeVec(rp));

    } catch (const std::exception& e) {
        RIPC_ERROR("ActionServer::handleResult", e.what());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Explicit instantiations  — add your Goal/Feedback/Result triples here.
//  Mirror this list in ActionClient.cpp and GoalHandle.cpp.
// ─────────────────────────────────────────────────────────────────────────────

} // namespace rcf
