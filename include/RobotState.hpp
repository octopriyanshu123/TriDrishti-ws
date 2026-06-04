#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <functional>
#include <nlohmann/json.hpp>


enum class RobotConnectionState { DISCONNECTED, ORIN_CONNECTED, NDT_CONNECTED, FULLY_CONNECTED };
enum class RobotRunState        { IDLE, INITIALISED, RUNNING, FROZEN, ERROR };
enum class OperationMode        { MANUAL, AUTOMATIC, SEMI_AUTO };
enum class InspectionSurface    { FLAT, CURVED, PIPE, CUSTOM };

struct RobotStateData
{
    std::string     lastCommand;          
    std::string     lastUpdatedAt;        
    uint64_t        commandCount { 0 };  

    struct Connection {
        RobotConnectionState state { RobotConnectionState::DISCONNECTED };
        bool orinConnected  { false };
        bool ndtConnected   { false };
    } connection;

    RobotRunState runState { RobotRunState::IDLE };

    struct SensorFlags {
        bool initialised { false };
        bool bypassed    { false };
        bool calibrated  { false };
    };
    struct Sensors {
        SensorFlags lidar;
        SensorFlags imu;
        SensorFlags wheel;
        SensorFlags laserProfiling;
        SensorFlags stm;
        bool        allCalibrated { false };
    } sensors;

    struct NDT {
        float gain      { 1.0f };
        float voltage   { 0.0f };
        int   filterId  { 0 };
    } ndt;

    struct Inspection {
        InspectionSurface surface        { InspectionSurface::FLAT };
        float             pipeDiaMm      { 0.0f };
        int               probCount      { 1 };
        float             rasterLengthMm { 0.0f };
        float             lengthMm       { 0.0f };
        float             customLengthMm { 0.0f };
    } inspection;

    struct InspectionRuntime {
        bool          running  { false };
        bool          frozen   { false };
        OperationMode mode     { OperationMode::MANUAL };
    } runtime;

    struct LastError {
        std::string source;
        std::string message;
        std::string timestamp;
    } lastError;
};