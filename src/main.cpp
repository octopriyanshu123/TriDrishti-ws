#include "Controllers/linearActuatorControllers.cpp"

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
        running = false;
    }
}

class Robot
{
public:
    Robot() = default;
    ~Robot()
    {
        std::cout << "[Robot] Cleanup complete\n";
    }

    void run()
    {
        spin();
        while (running)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

private:
    void spin()
    {
        ControllerManager manager(program_path);
    }

    std::string program_path =
        "/home/octo/Github/TriDrishti-ws/Sensors/build/linearActuator";
};

int main(int argc, char *argv[])
{
    std::signal(SIGINT, signalHandler);

    Robot robot;
    robot.run();
    std::cout << "[main] Exiting gracefully\n";
    return 0;
}