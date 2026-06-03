#include "Robot.hpp"
#include "StateManager.hpp"
#include "Logger.hpp"

#include <atomic>
#include <csignal>
#include <iostream>

std::atomic<bool> running{true};

static void onSignal(int) { running.store(false); }

void init()
{
    LOG_DEBUG("Main", "Starting up…");
    LOG_INFO("Main", "Robot initialised");
    LOG_WARNING("MotorController", "Left encoder jitter detected");
    LOG_ERROR("PowerModule", "Battery voltage critical");
}

int main()
{
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    Logger::getInstance().configure(Logger::LogLevel::INFO, "robot.log", true);

    Logger &log = Logger::getInstance();

    LOG_DEBUG("Main", "Starting up…");
    LOG_INFO("Main", "Robot initialised");
    LOG_WARNING("MotorController", "Left encoder jitter detected");
    LOG_ERROR("PowerModule", "Battery voltage critical");

    Robot robot;
    robot.run();
    return 0;
}