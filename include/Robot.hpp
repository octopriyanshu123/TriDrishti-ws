// #pragma once

// #include <string>
// #include <memory>
// #include <atomic>
// #include <functional>
// #include <iostream>
// #include <stdexcept>

// // ─────────────────────────────────────────────
// //  Forward declarations
// // ─────────────────────────────────────────────
// class ControllerManager;

// // ─────────────────────────────────────────────
// //  Enums
// // ─────────────────────────────────────────────
// enum class InspectionSurface
// {
//     FLAT,
//     CURVED,
//     PIPE,
//     CUSTOM
// };

// enum class OperationMode
// {
//     MANUAL,
//     AUTOMATIC,
//     SEMI_AUTO
// };

// enum class RobotState
// {
//     DISCONNECTED,
//     CONNECTED,
//     INITIALISED,
//     RUNNING,
//     FROZEN,
//     ERROR
// };

// // ─────────────────────────────────────────────
// //  Callback types
// // ─────────────────────────────────────────────
// using StatusCallback  = std::function<void(const std::string& component, bool ok)>;
// using ErrorCallback   = std::function<void(const std::string& source, const std::string& msg)>;
// using NDTDataCallback = std::function<void(float gain, float voltage, int filterId)>;

// // ─────────────────────────────────────────────
// //  Robot class
// // ─────────────────────────────────────────────
// class Robot
// {
// public:
//     // ── Lifecycle ──────────────────────────────
//     Robot(StateManager sm);
//     ~Robot();

//     Robot(const Robot&)            = delete;
//     Robot& operator=(const Robot&) = delete;

//     void run();
//     void stop();

//     RobotState getState() const;

//     // ── Callbacks ──────────────────────────────
//     void onStatus(StatusCallback cb);
//     void onError(ErrorCallback cb);
//     void onNDTData(NDTDataCallback cb);

//     // ============================================================
//     //  CONNECTION PAGE
//     // ============================================================
//     bool connectOrinNX();
//     bool connectNDT();
//     void disconnect();

//     // ============================================================
//     //  ROBOT SETTINGS  — init / reset / bypass
//     // ============================================================

//     // --- Lidar ---
//     bool initLidar();
//     bool resetLidar();
//     void bypassLidar(bool enable = true);

//     // --- IMU ---
//     bool initIMU();
//     bool resetIMU();
//     void bypassIMU(bool enable = true);

//     // --- Wheel ---
//     bool initWheel();
//     bool resetWheel();
//     void bypassWheel(bool enable = true);

//     // --- Laser Profiling ---
//     bool initLaserProfiling();
//     bool resetLaserProfiling();
//     void bypassLaserProfiling(bool enable = true);

//     // --- STM ---
//     bool initSTM();
//     bool resetSTM();
//     void bypassSTM(bool enable = true);

//     // --- Combined calibration ---
//     bool sensorCalibrate();

//     // ============================================================
//     //  NDT SETTINGS
//     // ============================================================
//     bool initNDTSettings();
//     bool resetNDTSettings();

//     // ============================================================
//     //  INSPECTION SETTINGS
//     // ============================================================
//     bool setInspectionSurface(InspectionSurface surface);
//     bool setInspectionPipeDia(float diameterMm);
//     bool setProbCount(int count);
//     bool setRasterLength(float lengthMm);
//     bool setInspectionLength(float lengthMm);
//     bool setInspectionCustomLength(float lengthMm);

//     // ============================================================
//     //  LIVE NDT  (Calibration controls)
//     // ============================================================
//     bool ndtGainIncrement();
//     bool ndtGainDecrement();
//     bool ndtSetFilterId(int filterId);
//     bool ndtSetVoltage(float voltage);

//     float ndtGetGain()    const;
//     float ndtGetVoltage() const;
//     int   ndtGetFilterId() const;

//     // ============================================================
//     //  INSPECTION PAGE  (runtime controls)
//     // ============================================================
//     bool btFreeze();
//     bool btResume();
//     bool btStart();
//     bool btAbort();
//     bool inspectionToggleOperationMode();

//     OperationMode getOperationMode() const;
//     bool          isInspectionRunning() const;
//     bool          isInspectionFrozen()  const;

// private:
//     // ── Internal helpers ───────────────────────
//     void log(const std::string& tag, const std::string& msg) const;
//     void reportError(const std::string& src, const std::string& msg);
//     bool requireState(RobotState required, const std::string& caller) const;
//     void setState(RobotState s);

//     // ── Members ────────────────────────────────
//     std::unique_ptr<ControllerManager> manager_;
//     const std::string program_path_ =
//         "/home/octobot/Github/TriDrishti-ws/Sensors/build/linearActuator";

//     std::atomic<RobotState> state_{ RobotState::DISCONNECTED };
//     std::atomic<bool>       inspectionRunning_{ false };
//     std::atomic<bool>       inspectionFrozen_{ false };

//     OperationMode operationMode_{ OperationMode::MANUAL };

//     // Bypass flags
//     bool bypassLidar_          { false };
//     bool bypassIMU_            { false };
//     bool bypassWheel_          { false };
//     bool bypassLaserProfiling_ { false };
//     bool bypassSTM_            { false };

//     // NDT live state
//     float ndtGain_    { 1.0f };
//     float ndtVoltage_ { 0.0f };
//     int   ndtFilterId_{ 0 };

//     // Inspection config
//     InspectionSurface inspectionSurface_{ InspectionSurface::FLAT };
//     float             pipeDiaMm_        { 0.0f };
//     int               probCount_        { 1 };
//     float             rasterLengthMm_   { 0.0f };
//     float             inspectionLenMm_  { 0.0f };
//     float             customLenMm_      { 0.0f };

//     // Callbacks
//     StatusCallback  statusCb_;
//     ErrorCallback   errorCb_;
//     NDTDataCallback ndtDataCb_;
// };

#pragma once

// ─────────────────────────────────────────────────────────────
//  Robot.hpp  —  Public interface for the TriDrishti Robot
//  All enums and RobotStateData live in StateManager.hpp.
//  Robot holds a reference to the single StateManager instance;
//  every API call mutates state and persists it to disk.
// ─────────────────────────────────────────────────────────────

#include "StateManager.hpp"   // RobotRunState, OperationMode,
                              // InspectionSurface, StateManager

#include <memory>
#include <string>
#include <atomic>

// Forward declaration — full type only needed in Robot.cpp
class ControllerManager;

// =============================================================
//  Robot
// =============================================================
class Robot
{
public:

    // ── Lifecycle ─────────────────────────────────────────────
    explicit Robot(StateManager& sm);
    ~Robot();

    Robot(const Robot&)            = delete;
    Robot& operator=(const Robot&) = delete;

    // Blocks until global `running` flag goes false (SIGINT/SIGTERM)
    void run();

    // Graceful shutdown — called by destructor and stop signal
    void stop();

    // ── State query ───────────────────────────────────────────
    RobotRunState  getState()            const;
    OperationMode  getOperationMode()    const;
    bool           isInspectionRunning() const;
    bool           isInspectionFrozen()  const;

    // ── Snapshot of the full persisted struct ─────────────────
    RobotStateData getStateData()        const;

    // ==========================================================
    //  CONNECTION PAGE
    // ==========================================================
    bool connectOrinNX();   // connect to Jetson Orin NX
    bool connectNDT();      // connect to NDT hardware unit
    void disconnect();      // tear down all connections

    // ==========================================================
    //  ROBOT SETTINGS  —  per-sensor init / reset / bypass
    // ==========================================================

    // --- Lidar ---
    bool initLidar();
    bool resetLidar();
    void bypassLidar(bool enable = true);

    // --- IMU ---
    bool initIMU();
    bool resetIMU();
    void bypassIMU(bool enable = true);

    // --- Wheel ---
    bool initWheel();
    bool resetWheel();
    void bypassWheel(bool enable = true);

    // --- Laser Profiling ---
    bool initLaserProfiling();
    bool resetLaserProfiling();
    void bypassLaserProfiling(bool enable = true);

    // --- STM microcontroller ---
    bool initSTM();
    bool resetSTM();
    void bypassSTM(bool enable = true);

    // --- Combined full-system calibration ---
    bool sensorCalibrate();

    // ==========================================================
    //  NDT SETTINGS
    // ==========================================================
    bool initNDTSettings();
    bool resetNDTSettings();

    // ==========================================================
    //  INSPECTION SETTINGS
    // ==========================================================
    bool setInspectionSurface(InspectionSurface surface);
    bool setInspectionPipeDia(float diameterMm);
    bool setProbCount(int count);
    bool setRasterLength(float lengthMm);
    bool setInspectionLength(float lengthMm);
    bool setInspectionCustomLength(float lengthMm);

    // ==========================================================
    //  LIVE NDT  —  real-time calibration controls
    // ==========================================================
    bool  ndtGainIncrement();           // gain += 0.1
    bool  ndtGainDecrement();           // gain -= 0.1  (floors at 0)
    bool  ndtSetFilterId(int filterId);
    bool  ndtSetVoltage(float voltage);

    // Live NDT getters  (read directly from persisted state)
    float ndtGetGain()     const;
    float ndtGetVoltage()  const;
    int   ndtGetFilterId() const;

    // ==========================================================
    //  INSPECTION PAGE  —  runtime controls
    // ==========================================================
    bool btStart();                         // begin inspection run
    bool btFreeze();                        // pause motion, keep data
    bool btResume();                        // resume from freeze
    bool btAbort();                         // safe-stop & discard run
    bool inspectionToggleOperationMode();   // cycles MANUAL → AUTO → SEMI_AUTO

private:

    // ── Internal helpers ──────────────────────────────────────
    void log(const std::string& tag, const std::string& msg) const;

    // ── Owned hardware manager ────────────────────────────────
    std::unique_ptr<ControllerManager> manager_;

    // Path to the external linearActuator binary
    const std::string program_path_ =
        "/home/octobot/Github/TriDrishti-ws/Sensors/build/linearActuator";

    // ── Shared state manager (injected, not owned) ────────────
    StateManager& sm_;
};