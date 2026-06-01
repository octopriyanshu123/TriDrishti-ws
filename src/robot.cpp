#include "Controllers/ControllerManager.cpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>

extern std::atomic<bool> running;

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
        // Manually Hiting the controller fopr ther it will hit by ui
        manager_ = std::make_unique<ControllerManager>(program_path_);
        manager_->init();
        std::cout << "[Robot] Initialising controllers...\n";

        while (running.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Graceful shutdown
        std::cout << "[Robot] Shutting down controllers...\n";
        manager_->shutdown();   // ← add this to ControllerManager
        manager_.reset();
        std::cout << "[Robot] All controllers stopped\n";
    }

private:
    std::unique_ptr<ControllerManager> manager_;

    const std::string program_path_ =
        "/home/octobot/Github/TriDrishti-ws/Sensors/build/linearActuator";
};