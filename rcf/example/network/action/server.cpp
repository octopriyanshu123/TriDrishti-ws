// =============================================================================
//  network/action/server.cpp  --  Robot side, TCP
//  One action: Navigate to (x, y)
//  No lambdas, no "using namespace", no chrono literals
//
//  Uses three consecutive TCP ports:
//    TCP_ACT_NAV+0  goal
//    TCP_ACT_NAV+1  cancel
//    TCP_ACT_NAV+2  result
// =============================================================================

#include "rcf.hpp"
#include "shared_types.hpp"

#include <cstdio>
#include <atomic>
#include <csignal>
#include <thread>
#include <chrono>

static std::atomic<bool> g_running{ true };

static bool check_nav(const NavGoal& g)
{
    bool safe = (g.x >= -20.f && g.x <= 20.f &&
                 g.y >= -20.f && g.y <= 20.f);
    if (!safe)
        printf("  [Nav] REJECTED (%.1f, %.1f)\n", g.x, g.y);
    return safe;
}

static void exec_nav(const NavGoal& goal,
                     rcf::ActionServer<NavGoal, NavFeedback, NavResult>::HandlePtr handle)
{
    printf("  [Nav] go to (%.1f, %.1f)\n", goal.x, goal.y);

    const int STEPS = 20;
    for (int i = 0; i < STEPS; ++i) {

        if (handle->isCancelRequested()) {
            printf("  [Nav] cancelled at step %d\n", i);
            NavResult r;
            r.reached = 0;
            r.final_x = goal.x * float(i) / STEPS;
            r.final_y = goal.y * float(i) / STEPS;
            handle->setCanceled(r);
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        NavFeedback fb;
        fb.progress = float(i + 1) / STEPS;
        fb.eta_sec  = float(STEPS - i - 1) * 0.1f;
        handle->publishFeedback(fb);

        printf("  [Nav] step %2d/%d  %.0f%%\n", i + 1, STEPS, fb.progress * 100.f);
    }

    NavResult r;
    r.reached = 1;
    r.final_x = goal.x;
    r.final_y = goal.y;
    handle->setSucceeded(r);
    printf("  [Nav] reached (%.1f, %.1f)\n", goal.x, goal.y);
}

int main()
{
    std::signal(SIGINT, [](int){ g_running = false; });

    // makeTcp(port_base) -- uses port_base, port_base+1, port_base+2
    auto srv = rcf::ActionServer<NavGoal, NavFeedback, NavResult>
                   ::makeTcp(TCP_ACT_NAV);

    srv->onGoalCheck(check_nav);
    srv->onExecute(exec_nav);
    srv->spinAsync();

    printf("Action server on TCP ports %d/%d/%d  --  Ctrl-C to stop\n\n",
           TCP_ACT_NAV, TCP_ACT_NAV + 1, TCP_ACT_NAV + 2);

    while (g_running)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    srv->shutdown();
    return 0;
}
