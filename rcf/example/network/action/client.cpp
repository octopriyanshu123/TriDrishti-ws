// =============================================================================
//  network/action/client.cpp  --  Control PC side, TCP
//  No lambdas, no "using namespace", no chrono literals
//
//  Usage:
//    ./network_action_client                  (connects to 127.0.0.1)
//    ./network_action_client 192.168.1.50     (connects to robot IP)
// =============================================================================

#include "rcf.hpp"
#include "shared_types.hpp"

#include <cstdio>
#include <string>
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

int main(int argc, char** argv)
{
    const std::string ip = (argc > 1) ? argv[1] : "127.0.0.1";
    printf("=== NETWORK ACTION CLIENT -> %s ports %d/%d/%d ===\n\n",
           ip.c_str(), TCP_ACT_NAV, TCP_ACT_NAV + 1, TCP_ACT_NAV + 2);

    // ActionClient<Goal, Feedback, Result>(host, port_base)  -- TCP constructor
    rcf::ActionClient<NavGoal, NavFeedback, NavResult> cli(ip, TCP_ACT_NAV);

    // ---- normal run ---------------------------------------------------------
    puts("-- normal run --");
    {
        NavGoal goal;
        goal.x = 5.f;
        goal.y = 3.f;

        rcf::ActionClient<NavGoal, NavFeedback, NavResult>::CGHPtr h =
            cli.sendGoal(goal, on_nav_feedback, on_nav_status);

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
