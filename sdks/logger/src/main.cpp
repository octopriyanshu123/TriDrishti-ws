
#include "logger.hpp"

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

// default folder ($HOME/log/robot.log)
Logger::getInstance().configure(Logger::LogLevel::DEBUG, "robot.log", false);

// Pass an absolute path directly as the filename, no logDir needed
Logger::getInstance().configure(Logger::LogLevel::DEBUG, "/home/octobot/robot/logs/robot.log", true);

 Logger &log = Logger::getInstance();

    LOG_DEBUG("Main", "Starting up…");


    LOG_INFO("Main", "Robot initialised");
    LOG_WARNING("MotorController", "Left encoder jitter detected");
    LOG_ERROR("PowerModule", "Battery voltage critical");
    return 0;
}
