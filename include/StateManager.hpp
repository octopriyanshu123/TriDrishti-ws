// #pragma once

// #include <string>
// #include <atomic>
// #include <mutex>
// #include <fstream>
// #include <iostream>
// #include <chrono>
// #include <ctime>
// #include <functional>
// #include <nlohmann/json.hpp>   

// using json = nlohmann::json;

// enum class RobotConnectionState { DISCONNECTED, ORIN_CONNECTED, NDT_CONNECTED, FULLY_CONNECTED };
// enum class RobotRunState        { IDLE, INITIALISED, RUNNING, FROZEN, ERROR };
// enum class OperationMode        { MANUAL, AUTOMATIC, SEMI_AUTO };
// enum class InspectionSurface    { FLAT, CURVED, PIPE, CUSTOM };

// // ─── string helpers ────────────────────────────────────────
// inline std::string toString(RobotConnectionState v) {
//     switch(v) {
//         case RobotConnectionState::DISCONNECTED:    return "DISCONNECTED";
//         case RobotConnectionState::ORIN_CONNECTED:  return "ORIN_CONNECTED";
//         case RobotConnectionState::NDT_CONNECTED:   return "NDT_CONNECTED";
//         case RobotConnectionState::FULLY_CONNECTED: return "FULLY_CONNECTED";
//     } return "UNKNOWN";
// }
// inline std::string toString(RobotRunState v) {
//     switch(v) {
//         case RobotRunState::IDLE:        return "IDLE";
//         case RobotRunState::INITIALISED: return "INITIALISED";
//         case RobotRunState::RUNNING:     return "RUNNING";
//         case RobotRunState::FROZEN:      return "FROZEN";
//         case RobotRunState::ERROR:       return "ERROR";
//     } return "UNKNOWN";
// }
// inline std::string toString(OperationMode v) {
//     switch(v) {
//         case OperationMode::MANUAL:     return "MANUAL";
//         case OperationMode::AUTOMATIC:  return "AUTOMATIC";
//         case OperationMode::SEMI_AUTO:  return "SEMI_AUTO";
//     } return "UNKNOWN";
// }
// inline std::string toString(InspectionSurface v) {
//     switch(v) {
//         case InspectionSurface::FLAT:   return "FLAT";
//         case InspectionSurface::CURVED: return "CURVED";
//         case InspectionSurface::PIPE:   return "PIPE";
//         case InspectionSurface::CUSTOM: return "CUSTOM";
//     } return "UNKNOWN";
// }


// struct RobotStateData
// {
//     std::string     lastUpdatedAt;        // ISO timestamp
//     uint64_t        commandCount { 0 };   // total commands received

//     // ── Connection ───────────────────────────────────────────
//     struct Connection {
//         RobotConnectionState state { RobotConnectionState::DISCONNECTED };
//         bool orinConnected  { false };
//         bool ndtConnected   { false };
//     } connection;

//     // ── Run state ────────────────────────────────────────────
//     RobotRunState runState { RobotRunState::IDLE };

//     // ── Sensor init flags ────────────────────────────────────
//     struct SensorFlags {
//         bool initialised { false };
//         bool bypassed    { false };
//         bool calibrated  { false };
//     };
//     struct Sensors {
//         SensorFlags lidar;
//         SensorFlags imu;
//         SensorFlags wheel;
//         SensorFlags laserProfiling;
//         SensorFlags stm;
//         bool        allCalibrated { false };
//     } sensors;

//     // ── NDT settings ─────────────────────────────────────────
//     struct NDT {
//         float gain      { 1.0f };
//         float voltage   { 0.0f };
//         int   filterId  { 0 };
//     } ndt;

//     // ── Inspection config ─────────────────────────────────────
//     struct Inspection {
//         InspectionSurface surface        { InspectionSurface::FLAT };
//         float             pipeDiaMm      { 0.0f };
//         int               probCount      { 1 };
//         float             rasterLengthMm { 0.0f };
//         float             lengthMm       { 0.0f };
//         float             customLengthMm { 0.0f };
//     } inspection;

//     // ── Inspection runtime ────────────────────────────────────
//     struct InspectionRuntime {
//         bool          running  { false };
//         bool          frozen   { false };
//         OperationMode mode     { OperationMode::MANUAL };
//     } runtime;

//     // ── Last error ────────────────────────────────────────────
//     struct LastError {
//         std::string source;
//         std::string message;
//         std::string timestamp;
//     } lastError;
// };


// inline json toJson(const RobotStateData& s)
// {
//     return {
//         {"meta", {
//             {"lastCommand",   s.lastCommand},
//             {"lastUpdatedAt", s.lastUpdatedAt},
//             {"commandCount",  s.commandCount}
//         }},
//         {"connection", {
//             {"state",          toString(s.connection.state)},
//             {"orinConnected",  s.connection.orinConnected},
//             {"ndtConnected",   s.connection.ndtConnected}
//         }},
//         {"runState", toString(s.runState)},
//         {"sensors", {
//             {"lidar",         {{"initialised", s.sensors.lidar.initialised},
//                                {"bypassed",    s.sensors.lidar.bypassed},
//                                {"calibrated",  s.sensors.lidar.calibrated}}},
//             {"imu",           {{"initialised", s.sensors.imu.initialised},
//                                {"bypassed",    s.sensors.imu.bypassed},
//                                {"calibrated",  s.sensors.imu.calibrated}}},
//             {"wheel",         {{"initialised", s.sensors.wheel.initialised},
//                                {"bypassed",    s.sensors.wheel.bypassed},
//                                {"calibrated",  s.sensors.wheel.calibrated}}},
//             {"laserProfiling",{{"initialised", s.sensors.laserProfiling.initialised},
//                                {"bypassed",    s.sensors.laserProfiling.bypassed},
//                                {"calibrated",  s.sensors.laserProfiling.calibrated}}},
//             {"stm",           {{"initialised", s.sensors.stm.initialised},
//                                {"bypassed",    s.sensors.stm.bypassed},
//                                {"calibrated",  s.sensors.stm.calibrated}}},
//             {"allCalibrated", s.sensors.allCalibrated}
//         }},
//         {"ndt", {
//             {"gain",     s.ndt.gain},
//             {"voltage",  s.ndt.voltage},
//             {"filterId", s.ndt.filterId}
//         }},
//         {"inspection", {
//             {"surface",        toString(s.inspection.surface)},
//             {"pipeDiaMm",      s.inspection.pipeDiaMm},
//             {"probCount",      s.inspection.probCount},
//             {"rasterLengthMm", s.inspection.rasterLengthMm},
//             {"lengthMm",       s.inspection.lengthMm},
//             {"customLengthMm", s.inspection.customLengthMm}
//         }},
//         {"runtime", {
//             {"running", s.runtime.running},
//             {"frozen",  s.runtime.frozen},
//             {"mode",    toString(s.runtime.mode)}
//         }},
//         {"lastError", {
//             {"source",    s.lastError.source},
//             {"message",   s.lastError.message},
//             {"timestamp", s.lastError.timestamp}
//         }}
//     };
// }

// // ============================================================
// //  STATE MANAGER
// // ============================================================

// class StateManager
// {
// public:
//     using OnChangeCallback = std::function<void(const RobotStateData&)>;

//     explicit StateManager(const std::string& filePath = "/home/octobot/log/robot_state.json")
//         : filePath_(filePath)
//     {
//         loadFromFile();   // restore last known state on startup
//     }

//     // ── Read (thread-safe snapshot) ────────────────────────────
//     RobotStateData get() const
//     {
//         std::lock_guard<std::mutex> lk(mutex_);
//         return state_;
//     }

//     // ── Register UI change listener ───────────────────────────
//     void onChange(OnChangeCallback cb) { onChangeCb_ = std::move(cb); }

//     // ============================================================
//     //  COMMAND HANDLERS  — called by Robot.cpp after each API call
//     // ============================================================

//     // ── Connection ───────────────────────────────────────────
//     void cmdConnectOrinNX(bool ok)
//     {
//         apply("connectOrinNX", [&](RobotStateData& s) {
//             s.connection.orinConnected = ok;
//             refreshConnectionState(s);
//         });
//     }
//     void cmdConnectNDT(bool ok)
//     {
//         apply("connectNDT", [&](RobotStateData& s) {
//             s.connection.ndtConnected = ok;
//             refreshConnectionState(s);
//         });
//     }
//     void cmdDisconnect()
//     {
//         apply("disconnect", [](RobotStateData& s) {
//             s.connection = {};
//             s.runState   = RobotRunState::IDLE;
//         });
//     }

//     // ── Sensor init ──────────────────────────────────────────
//     void cmdInitLidar(bool ok)          { apply("initLidar",          [&](RobotStateData& s){ s.sensors.lidar.initialised = ok; }); }
//     void cmdInitIMU(bool ok)            { apply("initIMU",            [&](RobotStateData& s){ s.sensors.imu.initialised   = ok; }); }
//     void cmdInitWheel(bool ok)          { apply("initWheel",          [&](RobotStateData& s){ s.sensors.wheel.initialised = ok; }); }
//     void cmdInitLaserProfiling(bool ok) { apply("initLaserProfiling", [&](RobotStateData& s){ s.sensors.laserProfiling.initialised = ok; }); }
//     void cmdInitSTM(bool ok)            { apply("initSTM",            [&](RobotStateData& s){ s.sensors.stm.initialised   = ok; }); }

//     // ── Sensor reset ─────────────────────────────────────────
//     void cmdResetLidar()          { apply("resetLidar",          [](RobotStateData& s){ s.sensors.lidar.initialised          = false; s.sensors.lidar.calibrated          = false; }); }
//     void cmdResetIMU()            { apply("resetIMU",            [](RobotStateData& s){ s.sensors.imu.initialised            = false; s.sensors.imu.calibrated            = false; }); }
//     void cmdResetWheel()          { apply("resetWheel",          [](RobotStateData& s){ s.sensors.wheel.initialised          = false; s.sensors.wheel.calibrated          = false; }); }
//     void cmdResetLaserProfiling() { apply("resetLaserProfiling", [](RobotStateData& s){ s.sensors.laserProfiling.initialised = false; s.sensors.laserProfiling.calibrated = false; }); }
//     void cmdResetSTM()            { apply("resetSTM",            [](RobotStateData& s){ s.sensors.stm.initialised            = false; s.sensors.stm.calibrated            = false; }); }

//     // ── Sensor bypass ────────────────────────────────────────
//     void cmdBypassLidar(bool en)          { apply("bypassLidar",          [&](RobotStateData& s){ s.sensors.lidar.bypassed          = en; }); }
//     void cmdBypassIMU(bool en)            { apply("bypassIMU",            [&](RobotStateData& s){ s.sensors.imu.bypassed            = en; }); }
//     void cmdBypassWheel(bool en)          { apply("bypassWheel",          [&](RobotStateData& s){ s.sensors.wheel.bypassed          = en; }); }
//     void cmdBypassLaserProfiling(bool en) { apply("bypassLaserProfiling", [&](RobotStateData& s){ s.sensors.laserProfiling.bypassed = en; }); }
//     void cmdBypassSTM(bool en)            { apply("bypassSTM",            [&](RobotStateData& s){ s.sensors.stm.bypassed            = en; }); }

//     // ── Calibration ──────────────────────────────────────────
//     void cmdSensorCalibrate(bool ok)
//     {
//         apply("sensorCalibrate", [&](RobotStateData& s) {
//             s.sensors.lidar.calibrated          = ok;
//             s.sensors.imu.calibrated            = ok;
//             s.sensors.wheel.calibrated          = ok;
//             s.sensors.laserProfiling.calibrated = ok;
//             s.sensors.stm.calibrated            = ok;
//             s.sensors.allCalibrated             = ok;
//             if (ok) s.runState = RobotRunState::INITIALISED;
//         });
//     }

//     // ── NDT ──────────────────────────────────────────────────
//     void cmdNdtGainIncrement(float newGain)    { apply("ndtGainIncrement", [&](RobotStateData& s){ s.ndt.gain     = newGain; }); }
//     void cmdNdtGainDecrement(float newGain)    { apply("ndtGainDecrement", [&](RobotStateData& s){ s.ndt.gain     = newGain; }); }
//     void cmdNdtSetFilterId(int id)             { apply("ndtSetFilterId",   [&](RobotStateData& s){ s.ndt.filterId = id;      }); }
//     void cmdNdtSetVoltage(float v)             { apply("ndtSetVoltage",    [&](RobotStateData& s){ s.ndt.voltage  = v;       }); }

//     // ── Inspection config ─────────────────────────────────────
//     void cmdSetInspectionSurface(InspectionSurface surf) { apply("setInspectionSurface", [&](RobotStateData& s){ s.inspection.surface        = surf; }); }
//     void cmdSetPipeDia(float mm)                         { apply("setInspectionPipeDia", [&](RobotStateData& s){ s.inspection.pipeDiaMm      = mm;   }); }
//     void cmdSetProbCount(int n)                          { apply("setProbCount",         [&](RobotStateData& s){ s.inspection.probCount       = n;    }); }
//     void cmdSetRasterLength(float mm)                    { apply("setRasterLength",      [&](RobotStateData& s){ s.inspection.rasterLengthMm  = mm;   }); }
//     void cmdSetInspectionLength(float mm)                { apply("setInspectionLength",  [&](RobotStateData& s){ s.inspection.lengthMm        = mm;   }); }
//     void cmdSetCustomLength(float mm)                    { apply("setCustomLength",      [&](RobotStateData& s){ s.inspection.customLengthMm  = mm;   }); }

//     // ── Inspection runtime ────────────────────────────────────
//     void cmdBtStart()
//     {
//         apply("btStart", [](RobotStateData& s) {
//             s.runtime.running = true;
//             s.runtime.frozen  = false;
//             s.runState        = RobotRunState::RUNNING;
//         });
//     }
//     void cmdBtFreeze()
//     {
//         apply("btFreeze", [](RobotStateData& s) {
//             s.runtime.frozen = true;
//             s.runState       = RobotRunState::FROZEN;
//         });
//     }
//     void cmdBtResume()
//     {
//         apply("btResume", [](RobotStateData& s) {
//             s.runtime.frozen = false;
//             s.runState       = RobotRunState::RUNNING;
//         });
//     }
//     void cmdBtAbort()
//     {
//         apply("btAbort", [](RobotStateData& s) {
//             s.runtime.running = false;
//             s.runtime.frozen  = false;
//             s.runState        = RobotRunState::IDLE;
//         });
//     }
//     void cmdToggleOperationMode(OperationMode newMode)
//     {
//         apply("toggleOperationMode", [&](RobotStateData& s) {
//             s.runtime.mode = newMode;
//         });
//     }

//     // ── Error ─────────────────────────────────────────────────
//     void cmdRecordError(const std::string& source, const std::string& message)
//     {
//         apply("error:" + source, [&](RobotStateData& s) {
//             s.lastError.source    = source;
//             s.lastError.message   = message;
//             s.lastError.timestamp = nowISO();
//             s.runState            = RobotRunState::ERROR;
//         });
//     }

// private:
//     // ── Core apply — mutate → timestamp → persist → notify ────
//     template<typename Fn>
//     void apply(const std::string& command, Fn&& mutate)
//     {
//         std::lock_guard<std::mutex> lk(mutex_);
//         mutate(state_);
//         state_.lastCommand   = command;
//         state_.lastUpdatedAt = nowISO();
//         state_.commandCount++;
//         saveToFile();
//         if (onChangeCb_) onChangeCb_(state_);
//         std::cout << "[StateManager] " << command
//                   << "  (#" << state_.commandCount << ")\n";
//     }

//     // ── Connection state machine ───────────────────────────────
//     static void refreshConnectionState(RobotStateData& s)
//     {
//         if (s.connection.orinConnected && s.connection.ndtConnected)
//             s.connection.state = RobotConnectionState::FULLY_CONNECTED;
//         else if (s.connection.orinConnected)
//             s.connection.state = RobotConnectionState::ORIN_CONNECTED;
//         else if (s.connection.ndtConnected)
//             s.connection.state = RobotConnectionState::NDT_CONNECTED;
//         else
//             s.connection.state = RobotConnectionState::DISCONNECTED;
//     }

//     // ── File I/O ──────────────────────────────────────────────
//     void saveToFile() const
//     {
//         std::ofstream f(filePath_);
//         if (!f.is_open()) {
//             std::cerr << "[StateManager] Cannot write: " << filePath_ << "\n";
//             return;
//         }
//         f << toJson(state_).dump(4);   // pretty-print, 4-space indent
//     }

//     void loadFromFile()
//     {
//         std::ifstream f(filePath_);
//         if (!f.is_open()) {
//             std::cout << "[StateManager] No existing state file — starting fresh\n";
//             return;
//         }
//         try {
//             json j = json::parse(f);
//             // restore meta
//             state_.lastCommand   = j["meta"].value("lastCommand",   "");
//             state_.lastUpdatedAt = j["meta"].value("lastUpdatedAt", "");
//             state_.commandCount  = j["meta"].value("commandCount",  uint64_t{0});
//             // restore NDT
//             state_.ndt.gain      = j["ndt"].value("gain",     1.0f);
//             state_.ndt.voltage   = j["ndt"].value("voltage",  0.0f);
//             state_.ndt.filterId  = j["ndt"].value("filterId", 0);
//             // restore inspection
//             state_.inspection.pipeDiaMm      = j["inspection"].value("pipeDiaMm",      0.0f);
//             state_.inspection.probCount      = j["inspection"].value("probCount",      1);
//             state_.inspection.rasterLengthMm = j["inspection"].value("rasterLengthMm", 0.0f);
//             state_.inspection.lengthMm       = j["inspection"].value("lengthMm",       0.0f);
//             state_.inspection.customLengthMm = j["inspection"].value("customLengthMm", 0.0f);
//             std::cout << "[StateManager] State restored from " << filePath_ << "\n";
//         } catch (const std::exception& e) {
//             std::cerr << "[StateManager] Parse error: " << e.what() << " — starting fresh\n";
//         }
//     }

//     // ── Timestamp ─────────────────────────────────────────────
//     static std::string nowISO()
//     {
//         auto now  = std::chrono::system_clock::now();
//         auto tt   = std::chrono::system_clock::to_time_t(now);
//         char buf[32];
//         std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&tt));
//         return std::string(buf);
//     }

//     mutable std::mutex mutex_;
//     RobotStateData     state_;
//     std::string        filePath_;
//     OnChangeCallback   onChangeCb_;
// };