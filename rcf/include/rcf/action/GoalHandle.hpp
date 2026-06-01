#pragma once
#include "rcf/Types.hpp"

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  GoalHandle<Feedback, Result>
//
//  Shared between the ActionServer's execute worker thread and the
//  result/cancel handler threads.  All synchronisation is internal.
//
//  Worker thread calls:   publishFeedback / isCancelRequested /
//                         setSucceeded / setAborted / setCanceled
//  Server calls:          transitionToExecuting / requestCancel
//  ActionClient calls:    waitForResult / getResult / status
//
//  Template bodies live in GoalHandle.cpp via explicit instantiation.
// ─────────────────────────────────────────────────────────────────────────────

namespace rcf {

template<typename Feedback, typename Result>
class GoalHandle {
public:
    using Ptr = std::shared_ptr<GoalHandle<Feedback, Result>>;

    explicit GoalHandle(GoalID id);
    ~GoalHandle();

    // ── Worker API (called from onExecute callback) ───────────────────────────
    void publishFeedback(const Feedback& fb);
    bool isCancelRequested() const;
    void setSucceeded(const Result& r);
    void setAborted  (const Result& r);
    void setCanceled (const Result& r = Result{});

    // ── Server-internal API ───────────────────────────────────────────────────
    void transitionToExecuting();
    void requestCancel();
    void setFeedbackCallback(std::function<void(const Feedback&)> cb);

    // ── Query API ─────────────────────────────────────────────────────────────
    GoalStatus status()     const;
    bool       isTerminal() const;
    bool       waitForResult(int timeout_ms = -1);
    Result     getResult()  const;
    GoalID     goalId()     const;

private:
    void transit(GoalStatus s);
    void store  (const Result& r);
    void notify ();

    GoalID                  id_;
    std::atomic<GoalStatus> status_;
    std::atomic<bool>       cancel_requested_;

    mutable std::mutex      result_mutex_;
    std::condition_variable result_cv_;
    Result                  result_{};
    bool                    result_ready_;

    std::mutex              fb_mutex_;
    Feedback                latest_fb_{};
    bool                    fb_updated_{false};
    std::function<void(const Feedback&)> fb_cb_;
};

} // namespace rcf
