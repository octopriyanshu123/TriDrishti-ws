#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <ctime>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "STMController.cpp"

class ControllerManager
{
public:
    explicit ControllerManager(const std::string &stm_driver_path)
        : stm_(stm_driver_path)
    {
    }

    void init()
    {

        std::cout << "[ControllerManager] init()\n";
        if (stm_.init() != 0)
        {
            controller_running = true;
            std::cout << "[ControllerManager] ready\n";
        }
        else
        {
            std::cout << "[ControllerManager] Stm init() Fail \n";
        }
    }

    void shutdown()
    {
        if (controller_running)
        {
            std::cout << "[ControllerManager] shutdown()\n";
            stm_.shutdown();
        }
    }

    ~ControllerManager()
    {
        shutdown();
    }

private:
    STMController stm_;
    pid_t stm_pid_;
    bool controller_running = false;
};
