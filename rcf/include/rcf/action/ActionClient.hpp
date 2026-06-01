#pragma once
#include "rcf/action/GoalHandle.hpp"
#include "rcf/transport/UnixTransport.hpp"
#include "rcf/transport/TcpTransport.hpp"
#include "rcf/Types.hpp"

#include <functional>
#include <memory>
#include <string>
#include <future>
#include <atomic>
#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
//  ClientGoalHandle<Feedback, Result>
//
//  Returned by ActionClient::sendGoal().  The caller uses it to:
//    • poll  getStatus()
//    • block waitForResult(timeout_ms)
//
//  Shared_ptr because both the caller and the result-waiter thread own it.
//  All method bodies are in ActionClient.cpp.
// ─────────────────────────────────────────────────────────────────────────────

namespace rcf {

template<typename Feedback, typename Result>
struct ClientGoalHandle {
    GoalID goal_id;

    // Current goal status — updated by the background result thread
    std::shared_ptr<std::atomic<GoalStatus>> status_atomic;

    // Promise/future pair: resolved exactly once when the result arrives
    std::shared_ptr<std::promise<std::pair<GoalStatus, Result>>> result_promise;
    std::shared_future<std::pair<GoalStatus, Result>>             result_future;

    ClientGoalHandle();   // body in .cpp

    GoalStatus getStatus() const;

    // Blocks until the goal reaches a terminal state (SUCCEEDED/CANCELED/ABORTED).
    // timeout_ms < 0 means wait forever.
    // Throws std::runtime_error("waitForResult timeout") on timeout.
    std::pair<GoalStatus, Result> waitForResult(int timeout_ms = -1);
};

// ─────────────────────────────────────────────────────────────────────────────
//  ActionClient<Goal, Feedback, Result>
//
//  Template declaration only — all method bodies are in ActionClient.cpp.
//  Mirrors the ActionServer socket layout:
//    offset 0  → goal     socket (Unix: <base>_goal,   TCP: port_base)
//    offset 1  → cancel   socket (Unix: <base>_cancel, TCP: port_base+1)
//    offset 2  → result   socket (Unix: <base>_result, TCP: port_base+2)
// ─────────────────────────────────────────────────────────────────────────────

template<typename Goal, typename Feedback, typename Result>
class ActionClient {
public:
    using CGH        = ClientGoalHandle<Feedback, Result>;
    using CGHPtr     = std::shared_ptr<CGH>;
    using FeedbackCb = std::function<void(const GoalID&, const Feedback&)>;
    using StatusCb   = std::function<void(const GoalID&, GoalStatus)>;

    // Unix-socket constructor  (same host)
    explicit ActionClient(const std::string& base, int timeout_ms = 5000);

    // TCP constructor  (remote host)
    ActionClient(const std::string& host, uint16_t port_base, int timeout_ms = 5000);

    // Send a goal; returns a shared handle immediately (non-blocking).
    // A background thread waits for the result and resolves the future.
    // fb_cb  — called on every publishFeedback() from the server (optional)
    // st_cb  — called once when the goal reaches a terminal state (optional)
    CGHPtr sendGoal(const Goal& goal,
                    FeedbackCb  fb_cb = nullptr,
                    StatusCb    st_cb = nullptr);

    // Send a cancel request for a running goal.
    // Returns true if the server acknowledged the cancel.
    bool cancel(CGHPtr cgh);

private:
    enum class Mode { UNIX, TCP };
    Mode        mode_;
    std::string base_;
    std::string host_;
    uint16_t    port_base_{0};
    int         timeout_ms_;

    // Open a fresh transport connection to one of the three channels.
    // offset: 0=goal, 1=cancel, 2=result
    std::unique_ptr<ITransport> connect(int offset) const;
};

} // namespace rcf
