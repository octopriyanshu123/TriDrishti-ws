#include "rcf.hpp"
#include "shared_types.hpp"

#include <cstdio>
#include <atomic>
#include <csignal>
#include <thread>
#include <chrono>

static std::atomic<bool> g_running{ true };

//  Goal check  -- return false to reject before execution starts
static bool check_nav(const NavGoal& g)
{
    bool safe = (g.x >= -20.f && g.x <= 20.f &&
                 g.y >= -20.f && g.y <= 20.f);
    if (!safe)
        printf("  [Nav] REJECTED (%.1f, %.1f) out of bounds\n", g.x, g.y);
    return safe;
}

//  Execute  -- runs in its own thread per accepted goal
//  handle->isCancelRequested()  -- check if client cancelled
//  handle->publishFeedback(fb)  -- send progress
//  handle->setSucceeded(result) -- mark done
//  handle->setCanceled(result)  -- acknowledge cancel

static void exec_nav(const NavGoal& goal,rcf::ActionServer<NavGoal, NavFeedback, NavResult>::HandlePtr handle)
{
    printf("  [Nav] go to (%.1f, %.1f)\n", goal.x, goal.y);

    const int STEPS = 20;
    for (int i = 0; i < STEPS; ++i) {

        if (handle->isCancelRequested()) {
            printf("  [Nav] cancelled at step %d/%d\n", i, STEPS);
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

        printf("  [Nav] step %2d/%d  %.0f%%  eta=%.1fs\n",
               i + 1, STEPS, fb.progress * 100.f, fb.eta_sec);
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

    auto srv = rcf::ActionServer<NavGoal, NavFeedback, NavResult>::makeUnix(ACT_NAV);

    srv->onGoalCheck(check_nav);
    srv->onExecute(exec_nav);
    srv->spinAsync();

    printf("Action server ready on %s_{goal,cancel,result}\n", ACT_NAV);
    printf("Ctrl-C to stop\n\n");

    while (g_running)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    srv->shutdown();
    return 0;
}
