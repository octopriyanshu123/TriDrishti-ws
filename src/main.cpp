// #include "Robot.hpp"

// #include <atomic>
// #include <csignal>
// #include <iostream>
// #include <thread>

// std::atomic<bool> running{true};

// void signalHandler(int signal)
// {
//     if (signal == SIGINT)
//     {
//         std::cout << "\n[main] Ctrl+C received. Shutting down...\n";
//         running = false; 
//     }
// }

// int main(int argc, char *argv[])
// {
//     std::signal(SIGINT, signalHandler);

//     Robot robot;
//     robot.run();

//     std::cout << "[main] Exiting gracefully\n";
//     return 0;
// }

#include "Robot.hpp"
#include "StateManager.hpp"

#include <atomic>
#include <csignal>
#include <iostream>

std::atomic<bool> running { true };

static void onSignal(int) { running.store(false); }

int main()
{
    std::signal(SIGINT,  onSignal);
    std::signal(SIGTERM, onSignal);

    // Single StateManager — owns the JSON file on disk
    StateManager sm("/var/log/tridrishti/robot_state.json");

    // Optional: react to every state change (e.g. push to UI via socket)
    sm.onChange([](const RobotStateData& s) {
        std::cout << "[State] cmd=" << s.lastCommand
                  << "  runState=" << toString(s.runState)
                  << "  #"         << s.commandCount << "\n";
    });

    // Robot holds a reference to the shared StateManager
    Robot robot(sm);
    robot.run();   // blocks until SIGINT

    return 0;
}