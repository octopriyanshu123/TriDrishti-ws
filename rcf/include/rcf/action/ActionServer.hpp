#pragma once
#include "rcf/action/GoalHandle.hpp"
#include "rcf/transport/ITransport.hpp"
#include "rcf/transport/UnixTransport.hpp"
#include "rcf/transport/TcpTransport.hpp"

#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <string>
#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
//  ActionServer<Goal, Feedback, Result>
//
//  Opens three sockets per action (goal / cancel / result).
//  All method bodies live in ActionServer.cpp.
// ─────────────────────────────────────────────────────────────────────────────

namespace rcf {

template<typename Goal, typename Feedback, typename Result>
class ActionServer {
public:
    using Handle    = GoalHandle<Feedback, Result>;
    using HandlePtr = typename Handle::Ptr;
    using ExecFn    = std::function<void(const Goal&, HandlePtr)>;
    using CheckFn   = std::function<bool(const Goal&)>;

    ActionServer() = default;
    ~ActionServer();

    // ── Factories ─────────────────────────────────────────────────────────────
    // Unix: creates <base>_goal, <base>_cancel, <base>_result
    static std::unique_ptr<ActionServer> makeUnix(const std::string& base);
    // TCP: uses ports port_base, port_base+1, port_base+2
    static std::unique_ptr<ActionServer> makeTcp(uint16_t port_base);

    // ── Setup ─────────────────────────────────────────────────────────────────
    void onExecute (ExecFn  f);   // mandatory: called in a new thread per goal
    void onGoalCheck(CheckFn f);  // optional:  return false to reject a goal

    // ── Lifecycle ─────────────────────────────────────────────────────────────
    void spin();       // blocks — runs all three accept loops
    void spinAsync();  // background thread
    void shutdown();

private:
    void startServers();
    void acceptLoop(ITransportServer* srv,
                    std::function<void(std::unique_ptr<ITransport>)> handler);
    void handleGoal  (std::unique_ptr<ITransport> t);
    void handleCancel(std::unique_ptr<ITransport> t);
    void handleResult(std::unique_ptr<ITransport> t);

    enum class Mode { UNIX, TCP };
    Mode        mode_{ Mode::UNIX };
    std::string base_;
    uint16_t    port_base_{ 0 };

    std::unique_ptr<ITransportServer> goal_srv_, cancel_srv_, result_srv_;
    ExecFn   exec_fn_;
    CheckFn  check_fn_;

    std::atomic<uint64_t>                  counter_{ 1 };
    std::mutex                             goals_mutex_;
    std::unordered_map<uint64_t, HandlePtr> goals_;
    std::atomic<bool>                      running_{ false };
    std::thread                            spin_thread_;
};

} // namespace rcf
