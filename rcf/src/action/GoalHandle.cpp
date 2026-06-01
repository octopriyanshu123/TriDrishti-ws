#include "rcf/action/GoalHandle.hpp"
#include "rcf/Logger.hpp"

#include <stdexcept>

namespace rcf {

template<typename Feedback, typename Result>
GoalHandle<Feedback, Result>::GoalHandle(GoalID id)
    : id_(id)
    , status_(GoalStatus::ACCEPTED)
    , cancel_requested_(false)
    , result_ready_(false)
{
    RIPC_DEBUG("GoalHandle", "Created " + id_.str());
}

template<typename Feedback, typename Result>
GoalHandle<Feedback, Result>::~GoalHandle() {
    RIPC_DEBUG("GoalHandle", "Destroyed " + id_.str());
}

// ── Worker API ────────────────────────────────────────────────────────────────
template<typename Feedback, typename Result>
void GoalHandle<Feedback, Result>::publishFeedback(const Feedback& fb) {
    std::lock_guard<std::mutex> lk(fb_mutex_);
    latest_fb_  = fb;
    fb_updated_ = true;
    if (fb_cb_) fb_cb_(fb);
}

template<typename Feedback, typename Result>
bool GoalHandle<Feedback, Result>::isCancelRequested() const {
    return cancel_requested_.load(std::memory_order_acquire);
}

template<typename Feedback, typename Result>
void GoalHandle<Feedback, Result>::setSucceeded(const Result& r) {
    transit(GoalStatus::SUCCEEDED); store(r); notify();
}

template<typename Feedback, typename Result>
void GoalHandle<Feedback, Result>::setAborted(const Result& r) {
    transit(GoalStatus::ABORTED); store(r); notify();
}

template<typename Feedback, typename Result>
void GoalHandle<Feedback, Result>::setCanceled(const Result& r) {
    transit(GoalStatus::CANCELED); store(r); notify();
}

// ── Server-internal API ───────────────────────────────────────────────────────
template<typename Feedback, typename Result>
void GoalHandle<Feedback, Result>::transitionToExecuting() {
    transit(GoalStatus::EXECUTING);
}

template<typename Feedback, typename Result>
void GoalHandle<Feedback, Result>::requestCancel() {
    auto s = status_.load();
    if (s != GoalStatus::EXECUTING && s != GoalStatus::ACCEPTED) return;
    transit(GoalStatus::CANCELING);
    cancel_requested_.store(true, std::memory_order_release);
    RIPC_INFO("GoalHandle", id_.str() + " cancel requested");
}

template<typename Feedback, typename Result>
void GoalHandle<Feedback, Result>::setFeedbackCallback(
    std::function<void(const Feedback&)> cb)
{
    std::lock_guard<std::mutex> lk(fb_mutex_);
    fb_cb_ = std::move(cb);
}

// ── Query API ─────────────────────────────────────────────────────────────────
template<typename Feedback, typename Result>
GoalStatus GoalHandle<Feedback, Result>::status() const {
    return status_.load(std::memory_order_acquire);
}

template<typename Feedback, typename Result>
bool GoalHandle<Feedback, Result>::isTerminal() const {
    auto s = status();
    return s == GoalStatus::SUCCEEDED ||
           s == GoalStatus::CANCELED  ||
           s == GoalStatus::ABORTED;
}

template<typename Feedback, typename Result>
bool GoalHandle<Feedback, Result>::waitForResult(int timeout_ms) {
    std::unique_lock<std::mutex> lk(result_mutex_);
    if (timeout_ms < 0) {
        result_cv_.wait(lk, [this] { return isTerminal(); });
        return true;
    }
    return result_cv_.wait_for(lk,
        std::chrono::milliseconds(timeout_ms),
        [this] { return isTerminal(); });
}

template<typename Feedback, typename Result>
Result GoalHandle<Feedback, Result>::getResult() const {
    std::lock_guard<std::mutex> lk(result_mutex_);
    if (!result_ready_) throw std::runtime_error("Result not ready");
    return result_;
}

template<typename Feedback, typename Result>
GoalID GoalHandle<Feedback, Result>::goalId() const { return id_; }

// ── Private helpers ───────────────────────────────────────────────────────────
template<typename Feedback, typename Result>
void GoalHandle<Feedback, Result>::transit(GoalStatus s) {
    status_.store(s, std::memory_order_release);
}

template<typename Feedback, typename Result>
void GoalHandle<Feedback, Result>::store(const Result& r) {
    std::lock_guard<std::mutex> lk(result_mutex_);
    result_       = r;
    result_ready_ = true;
}

template<typename Feedback, typename Result>
void GoalHandle<Feedback, Result>::notify() {
    result_cv_.notify_all();
    RIPC_INFO("GoalHandle", id_.str() + " -> " + goalStatusStr(status()));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Explicit instantiations — add your Goal/Feedback/Result triples here.
// ─────────────────────────────────────────────────────────────────────────────

} // namespace rcf
