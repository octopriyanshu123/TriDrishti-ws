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
    StateManager sm;

    LOG_DEBUG("Main", "Starting up…");
    LOG_INFO("Main", "Robot initialised");
    LOG_WARNING("MotorController", "Left encoder jitter detected");
    LOG_ERROR("PowerModule", "Battery voltage critical");
 // Load the settings from a JSON file (if it exists)
    RobotStateData state;
    sm.loadJSON(state, "robot_state_test.json");
    sm.printState(state);

    state.lastUpdatedAt = sm.nowISO();

    state.connection.state = RobotConnectionState::FULLY_CONNECTED;
    state.connection.orinConnected = true;
    state.connection.ndtConnected = true;
    state.runState = RobotRunState::INITIALISED;
    state.sensors.lidar.initialised = true;
    state.sensors.lidar.calibrated = true;
    state.sensors.imu.initialised = true;
    state.sensors.imu.calibrated = true;
    state.sensors.wheel.initialised = true;
    state.sensors.wheel.calibrated = true;
    state.sensors.laserProfiling.initialised = true;
    state.sensors.laserProfiling.calibrated = true;
    state.sensors.stm.initialised = true;
    state.sensors.stm.calibrated = true;
    state.sensors.allCalibrated = true;
    state.ndt.gain = 1.5f;
    state.ndt.voltage = 12.0f;
    state.ndt.filterId = 2;
    state.inspection.surface = InspectionSurface::PIPE;
    const std::string outFile = "robot_state_test.json";
    sm.printState(state);
    sm.saveJSON(state, outFile);

    Robot robot;
    robot.run();
    return 0;
}

