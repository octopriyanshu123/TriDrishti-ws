// // #include "Controllers/ControllerManager.cpp"

// // #include <chrono>
// // #include <iostream>
// // #include <memory>
// // #include <thread>
// // #include <atomic>

// // extern std::atomic<bool> running;

// // class Robot
// // {
// // public:
// //     Robot() = default;

// //     ~Robot()
// //     {
// //         std::cout << "[Robot] Cleanup complete\n";
// //     }

// //     void run()
// //     {
// //         // Manually Hiting the controller fopr ther it will hit by ui
// //         manager_ = std::make_unique<ControllerManager>(program_path_);
// //         manager_->init();
// //         std::cout << "[Robot] Initialising controllers...\n";

// //         while (running.load())
// //         {
// //             std::this_thread::sleep_for(std::chrono::milliseconds(100));
// //         }

// //         // Graceful shutdown
// //         std::cout << "[Robot] Shutting down controllers...\n";
// //         manager_->shutdown();   // ← add this to ControllerManager
// //         manager_.reset();
// //         std::cout << "[Robot] All controllers stopped\n";
// //     }

// // private:
// //     std::unique_ptr<ControllerManager> manager_;

// //     const std::string program_path_ =
// //         "/home/octobot/Github/TriDrishti-ws/Sensors/build/linearActuator";
// // };


// #include "Robot.hpp"
// #include "Controllers/ControllerManager.cpp"

// #include <chrono>
// #include <iostream>
// #include <thread>

// extern std::atomic<bool> running;

// Robot::Robot()
// {
//     log("Robot", "Instance created");
// }

// Robot::~Robot()
// {
//     stop();
//     log("Robot", "Cleanup complete");
// }

// void Robot::run()
// {
//     manager_ = std::make_unique<ControllerManager>(program_path_);
//     manager_->init();
//     setState(RobotState::INITIALISED);
//     log("Robot", "Controllers initialised, entering run loop");

//     while (running.load())
//     {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }

//     stop();
// }

// void Robot::stop()
// {
//     if (manager_)
//     {
//         log("Robot", "Shutting down controllers...");
//         manager_->shutdown();
//         manager_.reset();
//         log("Robot", "All controllers stopped");
//     }
//     setState(RobotState::DISCONNECTED);
// }

// RobotState Robot::getState() const
// {
//     return state_.load();
// }

// // ── Callbacks ──────────────────────────────────────────────

// void Robot::onStatus(StatusCallback cb)  { statusCb_  = std::move(cb); }
// void Robot::onError(ErrorCallback cb)    { errorCb_   = std::move(cb); }
// void Robot::onNDTData(NDTDataCallback cb){ ndtDataCb_ = std::move(cb); }


// bool Robot::connectOrinNX()
// {
//     log("Connection", "Connecting to Orin NX...");
//     try
//     {
//         // TODO: implement OrinNX transport handshake
//         //   e.g.  orinNxClient_.connect("192.168.1.10", 5000);
//         setState(RobotState::CONNECTED);
//         if (statusCb_) statusCb_("OrinNX", true);
//         log("Connection", "OrinNX connected");
//         return true;
//     }
//     catch (const std::exception& e)
//     {
//         reportError("connectOrinNX", e.what());
//         return false;
//     }
// }

// bool Robot::connectNDT()
// {
//     log("Connection", "Connecting to NDT unit...");
//     try
//     {
//         // TODO: implement NDT device handshake
//         //   e.g.  ndtSerial_.open("/dev/ttyUSB0", 115200);
//         if (statusCb_) statusCb_("NDT", true);
//         log("Connection", "NDT connected");
//         return true;
//     }
//     catch (const std::exception& e)
//     {
//         reportError("connectNDT", e.what());
//         return false;
//     }
// }

// void Robot::disconnect()
// {
//     log("Connection", "Disconnecting all...");
//     stop();
// }


// bool Robot::initLidar()
// {
//     if (bypassLidar_) { log("Lidar", "Bypassed — skipping init"); return true; }
//     log("Lidar", "Initialising...");
//     // TODO: manager_->lidar().init();
//     if (statusCb_) statusCb_("Lidar", true);
//     return true;
// }

// bool Robot::resetLidar()
// {
//     if (bypassLidar_) { log("Lidar", "Bypassed — skipping reset"); return true; }
//     log("Lidar", "Resetting...");
//     // TODO: manager_->lidar().reset();
//     if (statusCb_) statusCb_("Lidar", true);
//     return true;
// }

// void Robot::bypassLidar(bool enable)
// {
//     bypassLidar_ = enable;
//     log("Lidar", enable ? "Bypass ON" : "Bypass OFF");
// }


// bool Robot::initIMU()
// {
//     if (bypassIMU_) { log("IMU", "Bypassed — skipping init"); return true; }
//     log("IMU", "Initialising...");
//     // TODO: manager_->imu().init();
//     if (statusCb_) statusCb_("IMU", true);
//     return true;
// }

// bool Robot::resetIMU()
// {
//     if (bypassIMU_) { log("IMU", "Bypassed — skipping reset"); return true; }
//     log("IMU", "Resetting...");
//     // TODO: manager_->imu().reset();
//     if (statusCb_) statusCb_("IMU", true);
//     return true;
// }

// void Robot::bypassIMU(bool enable)
// {
//     bypassIMU_ = enable;
//     log("IMU", enable ? "Bypass ON" : "Bypass OFF");
// }

// // ── Wheel ──────────────────────────────────────────────────

// bool Robot::initWheel()
// {
//     if (bypassWheel_) { log("Wheel", "Bypassed — skipping init"); return true; }
//     log("Wheel", "Initialising...");
//     // TODO: manager_->wheel().init();
//     if (statusCb_) statusCb_("Wheel", true);
//     return true;
// }

// bool Robot::resetWheel()
// {
//     if (bypassWheel_) { log("Wheel", "Bypassed — skipping reset"); return true; }
//     log("Wheel", "Resetting...");
//     // TODO: manager_->wheel().reset();
//     if (statusCb_) statusCb_("Wheel", true);
//     return true;
// }

// void Robot::bypassWheel(bool enable)
// {
//     bypassWheel_ = enable;
//     log("Wheel", enable ? "Bypass ON" : "Bypass OFF");
// }

// // ── Laser Profiling ────────────────────────────────────────

// bool Robot::initLaserProfiling()
// {
//     if (bypassLaserProfiling_) { log("LaserProfiling", "Bypassed — skipping init"); return true; }
//     log("LaserProfiling", "Initialising...");
//     // TODO: manager_->laserProfiling().init();
//     if (statusCb_) statusCb_("LaserProfiling", true);
//     return true;
// }

// bool Robot::resetLaserProfiling()
// {
//     if (bypassLaserProfiling_) { log("LaserProfiling", "Bypassed — skipping reset"); return true; }
//     log("LaserProfiling", "Resetting...");
//     // TODO: manager_->laserProfiling().reset();
//     if (statusCb_) statusCb_("LaserProfiling", true);
//     return true;
// }

// void Robot::bypassLaserProfiling(bool enable)
// {
//     bypassLaserProfiling_ = enable;
//     log("LaserProfiling", enable ? "Bypass ON" : "Bypass OFF");
// }

// // ── STM ────────────────────────────────────────────────────

// bool Robot::initSTM()
// {
//     if (bypassSTM_) { log("STM", "Bypassed — skipping init"); return true; }
//     log("STM", "Initialising...");
//     // TODO: manager_->stm().init();
//     if (statusCb_) statusCb_("STM", true);
//     return true;
// }

// bool Robot::resetSTM()
// {
//     if (bypassSTM_) { log("STM", "Bypassed — skipping reset"); return true; }
//     log("STM", "Resetting...");
//     // TODO: manager_->stm().reset();
//     if (statusCb_) statusCb_("STM", true);
//     return true;
// }

// void Robot::bypassSTM(bool enable)
// {
//     bypassSTM_ = enable;
//     log("STM", enable ? "Bypass ON" : "Bypass OFF");
// }

// // ── Combined calibration ───────────────────────────────────

// bool Robot::sensorCalibrate()
// {
//     log("Calibration", "Starting full sensor calibration...");
//     bool ok = true;

//     if (!bypassLidar_)         ok &= initLidar();
//     if (!bypassIMU_)           ok &= initIMU();
//     if (!bypassWheel_)         ok &= initWheel();
//     if (!bypassLaserProfiling_)ok &= initLaserProfiling();
//     if (!bypassSTM_)           ok &= initSTM();

//     // TODO: call individual calibration routines on each sensor
//     //   e.g.  manager_->lidar().calibrate();

//     log("Calibration", ok ? "All sensors calibrated OK" : "Calibration completed with errors");
//     if (statusCb_) statusCb_("Calibration", ok);
//     return ok;
// }


// bool Robot::initNDTSettings()
// {
//     log("NDT", "Initialising NDT settings...");
//     // TODO: push default gain/voltage/filter to hardware
//     return true;
// }

// bool Robot::resetNDTSettings()
// {
//     ndtGain_     = 1.0f;
//     ndtVoltage_  = 0.0f;
//     ndtFilterId_ = 0;
//     log("NDT", "Settings reset to defaults");
//     return true;
// }

// // ── Inspection settings ────────────────────────────────────

// bool Robot::setInspectionSurface(InspectionSurface surface)
// {
//     inspectionSurface_ = surface;
//     log("Inspection", "Surface set to " + std::to_string(static_cast<int>(surface)));
//     // TODO: push to hardware / planner
//     return true;
// }

// bool Robot::setInspectionPipeDia(float diameterMm)
// {
//     if (diameterMm <= 0.0f)
//     {
//         reportError("setInspectionPipeDia", "Diameter must be > 0 mm");
//         return false;
//     }
//     pipeDiaMm_ = diameterMm;
//     log("Inspection", "Pipe diameter set to " + std::to_string(diameterMm) + " mm");
//     return true;
// }

// bool Robot::setProbCount(int count)
// {
//     if (count <= 0)
//     {
//         reportError("setProbCount", "Probe count must be > 0");
//         return false;
//     }
//     probCount_ = count;
//     log("Inspection", "Probe count set to " + std::to_string(count));
//     return true;
// }

// bool Robot::setRasterLength(float lengthMm)
// {
//     if (lengthMm <= 0.0f)
//     {
//         reportError("setRasterLength", "Raster length must be > 0 mm");
//         return false;
//     }
//     rasterLengthMm_ = lengthMm;
//     log("Inspection", "Raster length set to " + std::to_string(lengthMm) + " mm");
//     return true;
// }

// bool Robot::setInspectionLength(float lengthMm)
// {
//     if (lengthMm <= 0.0f)
//     {
//         reportError("setInspectionLength", "Inspection length must be > 0 mm");
//         return false;
//     }
//     inspectionLenMm_ = lengthMm;
//     log("Inspection", "Inspection length set to " + std::to_string(lengthMm) + " mm");
//     return true;
// }

// bool Robot::setInspectionCustomLength(float lengthMm)
// {
//     if (lengthMm <= 0.0f)
//     {
//         reportError("setInspectionCustomLength", "Custom length must be > 0 mm");
//         return false;
//     }
//     customLenMm_ = lengthMm;
//     log("Inspection", "Custom length set to " + std::to_string(lengthMm) + " mm");
//     return true;
// }


// // NDT  settings

// bool Robot::ndtGainIncrement()
// {
//     ndtGain_ += 0.1f;
//     log("NDT", "Gain incremented to " + std::to_string(ndtGain_));
//     if (ndtDataCb_) ndtDataCb_(ndtGain_, ndtVoltage_, ndtFilterId_);
//     // TODO: push new gain to hardware
//     return true;
// }

// bool Robot::ndtGainDecrement()
// {
//     if (ndtGain_ - 0.1f < 0.0f)
//     {
//         reportError("ndtGainDecrement", "Gain cannot go below 0");
//         return false;
//     }
//     ndtGain_ -= 0.1f;
//     log("NDT", "Gain decremented to " + std::to_string(ndtGain_));
//     if (ndtDataCb_) ndtDataCb_(ndtGain_, ndtVoltage_, ndtFilterId_);
//     return true;
// }

// bool Robot::ndtSetFilterId(int filterId)
// {
//     ndtFilterId_ = filterId;
//     log("NDT", "Filter ID set to " + std::to_string(filterId));
//     if (ndtDataCb_) ndtDataCb_(ndtGain_, ndtVoltage_, ndtFilterId_);
//     // TODO: push filter selection to hardware
//     return true;
// }

// bool Robot::ndtSetVoltage(float voltage)
// {
//     if (voltage < 0.0f)
//     {
//         reportError("ndtSetVoltage", "Voltage must be >= 0");
//         return false;
//     }
//     ndtVoltage_ = voltage;
//     log("NDT", "Voltage set to " + std::to_string(voltage) + " V");
//     if (ndtDataCb_) ndtDataCb_(ndtGain_, ndtVoltage_, ndtFilterId_);
//     return true;
// }

// float Robot::ndtGetGain()     const { return ndtGain_;     }
// float Robot::ndtGetVoltage()  const { return ndtVoltage_;  }
// int   Robot::ndtGetFilterId() const { return ndtFilterId_; }


// bool Robot::btFreeze()
// {
//     if (!inspectionRunning_.load())
//     {
//         reportError("btFreeze", "No active inspection to freeze");
//         return false;
//     }
//     inspectionFrozen_.store(true);
//     setState(RobotState::FROZEN);
//     log("Inspection", "FROZEN");
//     // TODO: halt motion controller but keep data pipeline alive
//     return true;
// }

// bool Robot::btResume()
// {
//     if (!inspectionFrozen_.load())
//     {
//         reportError("btResume", "Inspection is not frozen");
//         return false;
//     }
//     inspectionFrozen_.store(false);
//     setState(RobotState::RUNNING);
//     log("Inspection", "RESUMED");
//     // TODO: restart motion controller
//     return true;
// }

// bool Robot::btStart()
// {
//     if (inspectionRunning_.load())
//     {
//         reportError("btStart", "Inspection already running");
//         return false;
//     }
//     log("Inspection", "STARTING...");
//     // TODO: validate all inspection params, arm sensors, start acquisition
//     //   e.g.  manager_->acquisitionPipeline().start();
//     inspectionRunning_.store(true);
//     inspectionFrozen_.store(false);
//     setState(RobotState::RUNNING);
//     log("Inspection", "RUNNING");
//     return true;
// }

// bool Robot::btAbort()
// {
//     if (!inspectionRunning_.load())
//     {
//         reportError("btAbort", "No active inspection to abort");
//         return false;
//     }
//     log("Inspection", "ABORTING...");
//     // TODO: safe-stop motion, flush buffers, save partial data
//     //   e.g.  manager_->acquisitionPipeline().stop();
//     inspectionRunning_.store(false);
//     inspectionFrozen_.store(false);
//     setState(RobotState::CONNECTED);
//     log("Inspection", "ABORTED");
//     return true;
// }

// bool Robot::inspectionToggleOperationMode()
// {
//     if (operationMode_ == OperationMode::MANUAL)
//     {
//         operationMode_ = OperationMode::AUTOMATIC;
//         log("Inspection", "Mode → AUTOMATIC");
//     }
//     else if (operationMode_ == OperationMode::AUTOMATIC)
//     {
//         operationMode_ = OperationMode::SEMI_AUTO;
//         log("Inspection", "Mode → SEMI_AUTO");
//     }
//     else
//     {
//         operationMode_ = OperationMode::MANUAL;
//         log("Inspection", "Mode → MANUAL");
//     }
//     // TODO: push mode change to motion planner
//     return true;
// }

// OperationMode Robot::getOperationMode()    const { return operationMode_;           }
// bool          Robot::isInspectionRunning() const { return inspectionRunning_.load(); }
// bool          Robot::isInspectionFrozen()  const { return inspectionFrozen_.load();  }


// void Robot::log(const std::string& tag, const std::string& msg) const
// {
//     std::cout << "[" << tag << "] " << msg << "\n";
// }

// void Robot::reportError(const std::string& src, const std::string& msg)
// {
//     std::cerr << "[ERROR][" << src << "] " << msg << "\n";
//     if (errorCb_) errorCb_(src, msg);
// }

// bool Robot::requireState(RobotState required, const std::string& caller) const
// {
//     if (state_.load() != required)
//     {
//         std::cerr << "[ERROR][" << caller << "] Wrong state ("
//                   << static_cast<int>(state_.load()) << ")\n";
//         return false;
//     }
//     return true;
// }

// void Robot::setState(RobotState s)

// {
//     state_.store(s);
// }


#include "Robot.hpp"
#include "StateManager.hpp"
#include "Controllers/ControllerManager.cpp"

#include <chrono>
#include <iostream>
#include <thread>

extern std::atomic<bool> running;

// ── Constructor takes StateManager by reference ────────────

Robot::Robot(StateManager& sm) : sm_(sm)
{
    log("Robot", "Instance created");
}

Robot::~Robot() { stop(); log("Robot", "Cleanup complete"); }

void Robot::run()
{
    manager_ = std::make_unique<ControllerManager>(program_path_);
    manager_->init();
    sm_.cmdSensorCalibrate(true);
    log("Robot", "Entering run loop");
    while (running.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop();
}

void Robot::stop()
{
    if (manager_) { manager_->shutdown(); manager_.reset(); }
    sm_.cmdDisconnect();
}

// ── CONNECTION ─────────────────────────────────────────────

bool Robot::connectOrinNX()
{
    bool ok = true;   // TODO: real transport
    sm_.cmdConnectOrinNX(ok);
    return ok;
}

bool Robot::connectNDT()
{
    bool ok = true;   // TODO: real transport
    sm_.cmdConnectNDT(ok);
    return ok;
}

void Robot::disconnect() { stop(); }

// ── SENSOR INIT ────────────────────────────────────────────

bool Robot::initLidar()          { bool ok = !sm_.get().sensors.lidar.bypassed;          sm_.cmdInitLidar(ok);          return ok; }
bool Robot::initIMU()            { bool ok = !sm_.get().sensors.imu.bypassed;            sm_.cmdInitIMU(ok);            return ok; }
bool Robot::initWheel()          { bool ok = !sm_.get().sensors.wheel.bypassed;          sm_.cmdInitWheel(ok);          return ok; }
bool Robot::initLaserProfiling() { bool ok = !sm_.get().sensors.laserProfiling.bypassed; sm_.cmdInitLaserProfiling(ok); return ok; }
bool Robot::initSTM()            { bool ok = !sm_.get().sensors.stm.bypassed;            sm_.cmdInitSTM(ok);            return ok; }

// ── SENSOR RESET ───────────────────────────────────────────

bool Robot::resetLidar()          { sm_.cmdResetLidar();          return true; }
bool Robot::resetIMU()            { sm_.cmdResetIMU();            return true; }
bool Robot::resetWheel()          { sm_.cmdResetWheel();          return true; }
bool Robot::resetLaserProfiling() { sm_.cmdResetLaserProfiling(); return true; }
bool Robot::resetSTM()            { sm_.cmdResetSTM();            return true; }

// ── SENSOR BYPASS ──────────────────────────────────────────

void Robot::bypassLidar(bool en)          { sm_.cmdBypassLidar(en);          }
void Robot::bypassIMU(bool en)            { sm_.cmdBypassIMU(en);            }
void Robot::bypassWheel(bool en)          { sm_.cmdBypassWheel(en);          }
void Robot::bypassLaserProfiling(bool en) { sm_.cmdBypassLaserProfiling(en); }
void Robot::bypassSTM(bool en)            { sm_.cmdBypassSTM(en);            }

// ── CALIBRATE ─────────────────────────────────────────────

bool Robot::sensorCalibrate()
{
    bool ok = initLidar() & initIMU() & initWheel() & initLaserProfiling() & initSTM();
    sm_.cmdSensorCalibrate(ok);
    return ok;
}

// ── INSPECTION CONFIG ──────────────────────────────────────

bool Robot::setInspectionSurface(InspectionSurface s)  { sm_.cmdSetInspectionSurface(s); return true; }
bool Robot::setInspectionPipeDia(float mm)             { if(mm<=0){sm_.cmdRecordError("setPipeDia","<=0");    return false;} sm_.cmdSetPipeDia(mm);          return true; }
bool Robot::setProbCount(int n)                        { if(n<=0) {sm_.cmdRecordError("setProbCount","<=0");  return false;} sm_.cmdSetProbCount(n);         return true; }
bool Robot::setRasterLength(float mm)                  { if(mm<=0){sm_.cmdRecordError("setRaster","<=0");     return false;} sm_.cmdSetRasterLength(mm);     return true; }
bool Robot::setInspectionLength(float mm)              { if(mm<=0){sm_.cmdRecordError("setLength","<=0");     return false;} sm_.cmdSetInspectionLength(mm); return true; }
bool Robot::setInspectionCustomLength(float mm)        { if(mm<=0){sm_.cmdRecordError("setCustomLen","<=0");  return false;} sm_.cmdSetCustomLength(mm);     return true; }

// ── NDT LIVE ───────────────────────────────────────────────

bool Robot::ndtGainIncrement()
{
    float g = sm_.get().ndt.gain + 0.1f;
    sm_.cmdNdtGainIncrement(g);
    return true;
}

bool Robot::ndtGainDecrement()
{
    float g = sm_.get().ndt.gain - 0.1f;
    if (g < 0.0f) { sm_.cmdRecordError("ndtGainDecrement","below 0"); return false; }
    sm_.cmdNdtGainDecrement(g);
    return true;
}

bool Robot::ndtSetFilterId(int id) { sm_.cmdNdtSetFilterId(id); return true; }
bool Robot::ndtSetVoltage(float v) { if(v<0){sm_.cmdRecordError("ndtSetVoltage","<0"); return false;} sm_.cmdNdtSetVoltage(v); return true; }

float Robot::ndtGetGain()     const { return sm_.get().ndt.gain;     }
float Robot::ndtGetVoltage()  const { return sm_.get().ndt.voltage;  }
int   Robot::ndtGetFilterId() const { return sm_.get().ndt.filterId; }

// ── INSPECTION RUNTIME ─────────────────────────────────────

bool Robot::btStart()
{
    if (sm_.get().runtime.running) { sm_.cmdRecordError("btStart","already running"); return false; }
    sm_.cmdBtStart();
    return true;
}

bool Robot::btFreeze()
{
    if (!sm_.get().runtime.running) { sm_.cmdRecordError("btFreeze","not running"); return false; }
    sm_.cmdBtFreeze();
    return true;
}

bool Robot::btResume()
{
    if (!sm_.get().runtime.frozen) { sm_.cmdRecordError("btResume","not frozen"); return false; }
    sm_.cmdBtResume();
    return true;
}

bool Robot::btAbort()
{
    if (!sm_.get().runtime.running) { sm_.cmdRecordError("btAbort","not running"); return false; }
    sm_.cmdBtAbort();
    return true;
}

bool Robot::inspectionToggleOperationMode()
{
    auto cur = sm_.get().runtime.mode;
    OperationMode next =
        cur == OperationMode::MANUAL    ? OperationMode::AUTOMATIC :
        cur == OperationMode::AUTOMATIC ? OperationMode::SEMI_AUTO :
                                          OperationMode::MANUAL;
    sm_.cmdToggleOperationMode(next);
    return true;
}

OperationMode Robot::getOperationMode()    const { return sm_.get().runtime.mode;    }
bool          Robot::isInspectionRunning() const { return sm_.get().runtime.running; }
bool          Robot::isInspectionFrozen()  const { return sm_.get().runtime.frozen;  }
RobotRunState Robot::getState()            const { return sm_.get().runState;         }

// ── Helpers ────────────────────────────────────────────────

void Robot::log(const std::string& tag, const std::string& msg) const
{
    std::cout << "[" << tag << "] " << msg << "\n";
}