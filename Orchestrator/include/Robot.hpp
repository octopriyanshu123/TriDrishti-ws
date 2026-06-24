#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <functional>
#include <iostream>
#include <stdexcept>
#include "RobotState.hpp"

// ─────────────────────────────────────────────
//  Forward declarations
// ─────────────────────────────────────────────
class ControllerManager;

// ─────────────────────────────────────────────
//  Enums
// ─────────────────────────────────────────────

enum class RobotState
{
    DISCONNECTED,
    CONNECTED,
    INITIALISED,
    RUNNING,
    FROZEN,
    ERROR
};

// ─────────────────────────────────────────────
//  Callback types
// ─────────────────────────────────────────────
using StatusCallback  = std::function<void(const std::string& component, bool ok)>;
using ErrorCallback   = std::function<void(const std::string& source, const std::string& msg)>;
using NDTDataCallback = std::function<void(float gain, float voltage, int filterId)>;

// ─────────────────────────────────────────────
//  Robot class
// ─────────────────────────────────────────────
class Robot
{
public:
    // ── Lifecycle ──────────────────────────────
    Robot();
    ~Robot();

    Robot(const Robot&)            = delete;
    Robot& operator=(const Robot&) = delete;

    void run();
    void stop();

    RobotState getState() const;

    // ── Callbacks ──────────────────────────────
    void onStatus(StatusCallback cb);
    void onError(ErrorCallback cb);
    void onNDTData(NDTDataCallback cb);

    // ============================================================
    //  CONNECTION PAGE
    // ============================================================
    bool connectOrinNX();
    bool connectNDT();
    void disconnect();

    // ============================================================
    //  ROBOT SETTINGS  — init / reset / bypass
    // ============================================================

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

    // --- STM ---
    bool initSTM();
    bool resetSTM();
    void bypassSTM(bool enable = true);

    // --- Combined calibration ---
    bool sensorCalibrate();

    // ============================================================
    //  NDT SETTINGS
    // ============================================================
    bool initNDTSettings();
    bool resetNDTSettings();

    // ============================================================
    //  INSPECTION SETTINGS
    // ============================================================
    bool setInspectionSurface(InspectionSurface surface);
    bool setInspectionPipeDia(float diameterMm);
    bool setProbCount(int count);
    bool setRasterLength(float lengthMm);
    bool setInspectionLength(float lengthMm);
    bool setInspectionCustomLength(float lengthMm);

    // ============================================================
    //  LIVE NDT  (Calibration controls)
    // ============================================================
    bool ndtGainIncrement();
    bool ndtGainDecrement();
    bool ndtSetFilterId(int filterId);
    bool ndtSetVoltage(float voltage);

    float ndtGetGain()    const;
    float ndtGetVoltage() const;
    int   ndtGetFilterId() const;

    // ============================================================
    //  INSPECTION PAGE  (runtime controls)
    // ============================================================
    bool btFreeze();
    bool btResume();
    bool btStart();
    bool btAbort();
    bool inspectionToggleOperationMode();

    OperationMode getOperationMode() const;
    bool          isInspectionRunning() const;
    bool          isInspectionFrozen()  const;

private:
    // ── Internal helpers ───────────────────────
    void log(const std::string& tag, const std::string& msg) const;
    void reportError(const std::string& src, const std::string& msg);
    bool requireState(RobotState required, const std::string& caller) const;
    void setState(RobotState s);

    // ── Members ────────────────────────────────
    std::unique_ptr<ControllerManager> manager_;
    const std::string program_path_ =
        "/home/octobot/Github/TriDrishti-ws/Sensors/build/linearActuator";

    std::atomic<RobotState> state_{ RobotState::DISCONNECTED };
    std::atomic<bool>       inspectionRunning_{ false };
    std::atomic<bool>       inspectionFrozen_{ false };

    OperationMode operationMode_{ OperationMode::MANUAL };

    // Bypass flags
    bool bypassLidar_          { false };
    bool bypassIMU_            { false };
    bool bypassWheel_          { false };
    bool bypassLaserProfiling_ { false };
    bool bypassSTM_            { false };

    // NDT live state
    float ndtGain_    { 1.0f };
    float ndtVoltage_ { 0.0f };
    int   ndtFilterId_{ 0 };

    // Inspection config
    InspectionSurface inspectionSurface_{ InspectionSurface::FLAT };
    float             pipeDiaMm_        { 0.0f };
    int               probCount_        { 1 };
    float             rasterLengthMm_   { 0.0f };
    float             inspectionLenMm_  { 0.0f };
    float             customLenMm_      { 0.0f };

    // Callbacks
    StatusCallback  statusCb_;
    ErrorCallback   errorCb_;
    NDTDataCallback ndtDataCb_;
};

#pragma once

