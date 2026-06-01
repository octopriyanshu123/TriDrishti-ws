#include "robot.cpp"

#include <atomic>
#include <csignal>
#include <iostream>
#include <thread>

std::atomic<bool> running{true};

void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        std::cout << "\n[main] Ctrl+C received. Shutting down...\n";
        running = false;  // ← was missing!
    }
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT, signalHandler);

    Robot robot;
    robot.run();

    std::cout << "[main] Exiting gracefully\n";
    return 0;
}