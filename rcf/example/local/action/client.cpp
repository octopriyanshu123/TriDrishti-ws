#include "rcf.hpp"
#include "shared_types.hpp"
#include <cstdio>
#include <thread>
#include <chrono>

static void on_nav_feedback(const rcf::GoalID& id, const NavFeedback& fb)
{
    printf("  [fb] %s  %.0f%%  eta=%.1fs\n",
           id.str().c_str(), fb.progress * 100.f, fb.eta_sec);
}

static void on_nav_status(const rcf::GoalID& id, rcf::GoalStatus s)
{
    printf("  [status] %s -> %s\n", id.str().c_str(), rcf::goalStatusStr(s));
}

int main()
{
    puts("=== LOCAL ACTION CLIENT ===");

    rcf::ActionClient<NavGoal, NavFeedback, NavResult> cli(ACT_NAV);

    puts("\n-- normal run --");
    {
        NavGoal goal;
        goal.x = 5.f;
        goal.y = 3.f;

        // sendGoal returns a handle immediately (non-blocking)
        // the result-waiter runs in a background thread
        rcf::ActionClient<NavGoal, NavFeedback, NavResult>::CGHPtr h =
            cli.sendGoal(goal, on_nav_feedback, on_nav_status);

        // waitForResult(timeout_ms) blocks until SUCCEEDED / CANCELED / ABORTED
        std::pair<rcf::GoalStatus, NavResult> r = h->waitForResult(15000);
        printf("  status=%-10s  reached=%s  pos=(%.1f,%.1f)\n",
               rcf::goalStatusStr(r.first),
               r.second.reached ? "yes" : "no",
               r.second.final_x, r.second.final_y);
    }

    // ---- cancel mid-flight --------------------------------------------------
    puts("\n-- cancel mid-flight --");
    {
        NavGoal goal;
        goal.x = 10.f;
        goal.y = 8.f;

        rcf::ActionClient<NavGoal, NavFeedback, NavResult>::CGHPtr h =
            cli.sendGoal(goal, on_nav_feedback);

        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        printf("  sending cancel...\n");
        bool ack = cli.cancel(h);
        printf("  cancel acked: %s\n", ack ? "yes" : "no");

        std::pair<rcf::GoalStatus, NavResult> r = h->waitForResult(5000);
        printf("  status=%-10s  reached=%s  pos=(%.1f,%.1f)\n",
               rcf::goalStatusStr(r.first),
               r.second.reached ? "yes" : "no",
               r.second.final_x, r.second.final_y);
    }

    // ---- rejected goal ------------------------------------------------------
    puts("\n-- rejected goal (out of bounds) --");
    {
        NavGoal goal;
        goal.x = 99.f;
        goal.y = 99.f;

        rcf::ActionClient<NavGoal, NavFeedback, NavResult>::CGHPtr h =
            cli.sendGoal(goal);

        std::pair<rcf::GoalStatus, NavResult> r = h->waitForResult(3000);
        printf("  status=%s (expected REJECTED)\n", rcf::goalStatusStr(r.first));
    }

    puts("\nDone.");
    return 0;
}
