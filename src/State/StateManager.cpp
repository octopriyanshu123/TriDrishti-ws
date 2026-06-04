#include "StateManager.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <limits>


static std::string nowISO()
{
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&tt));
    return buf;
}

static void printBool(const std::string &label, bool v)
{
    std::cout << "    " << std::left << std::setw(22) << label
              << (v ? "YES" : "NO") << "\n";
}

static void printSensorBlock(const std::string &name,
                             const RobotStateData::SensorFlags &f)
{
    std::cout << "  [" << name << "]\n";
    printBool("initialised:", f.initialised);
    printBool("bypassed:", f.bypassed);
    printBool("calibrated:", f.calibrated);
}

// ─────────────────────────────────────────────
//  Manual JSON serialiser  (no third-party lib)
// ─────────────────────────────────────────────

static std::string boolStr(bool v) { return v ? "true" : "false"; }
static std::string quoted(const std::string &s) { return "\"" + s + "\""; }

static std::string connStateStr(RobotConnectionState s)
{
    switch (s)
    {
    case RobotConnectionState::DISCONNECTED:
        return "DISCONNECTED";
    case RobotConnectionState::ORIN_CONNECTED:
        return "ORIN_CONNECTED";
    case RobotConnectionState::NDT_CONNECTED:
        return "NDT_CONNECTED";
    case RobotConnectionState::FULLY_CONNECTED:
        return "FULLY_CONNECTED";
    }
    return "UNKNOWN";
}
static std::string runStateStr(RobotRunState s)
{
    switch (s)
    {
    case RobotRunState::IDLE:
        return "IDLE";
    case RobotRunState::INITIALISED:
        return "INITIALISED";
    case RobotRunState::RUNNING:
        return "RUNNING";
    case RobotRunState::FROZEN:
        return "FROZEN";
    case RobotRunState::ERROR:
        return "ERROR";
    }
    return "UNKNOWN";
}
static std::string opModeStr(OperationMode m)
{
    switch (m)
    {
    case OperationMode::MANUAL:
        return "MANUAL";
    case OperationMode::AUTOMATIC:
        return "AUTOMATIC";
    case OperationMode::SEMI_AUTO:
        return "SEMI_AUTO";
    }
    return "UNKNOWN";
}
static std::string surfaceStr(InspectionSurface s)
{
    switch (s)
    {
    case InspectionSurface::FLAT:
        return "FLAT";
    case InspectionSurface::CURVED:
        return "CURVED";
    case InspectionSurface::PIPE:
        return "PIPE";
    case InspectionSurface::CUSTOM:
        return "CUSTOM";
    }
    return "UNKNOWN";
}

static std::string sensorJson(const std::string &indent,
                              const RobotStateData::SensorFlags &f)
{
    return indent + "{ \"initialised\": " + boolStr(f.initialised) + ", \"bypassed\": " + boolStr(f.bypassed) + ", \"calibrated\": " + boolStr(f.calibrated) + " }";
}

static void saveJSON(const RobotStateData &s, const std::string &path)
{
    std::ofstream f(path);
    if (!f)
    {
        std::cerr << "Cannot open " << path << "\n";
        return;
    }

    f << std::fixed << std::setprecision(4);
    f << "{\n";

    // meta
    f << "  \"lastUpdatedAt\": " << quoted(s.lastUpdatedAt) << ",\n";
    f << "  \"commandCount\": " << s.commandCount << ",\n";

    // connection
    f << "  \"connection\": {\n";
    f << "    \"state\": " << quoted(connStateStr(s.connection.state)) << ",\n";
    f << "    \"orinConnected\": " << boolStr(s.connection.orinConnected) << ",\n";
    f << "    \"ndtConnected\": " << boolStr(s.connection.ndtConnected) << "\n";
    f << "  },\n";

    // runState
    f << "  \"runState\": " << quoted(runStateStr(s.runState)) << ",\n";

    // sensors
    f << "  \"sensors\": {\n";
    f << "    \"lidar\": " << sensorJson("", s.sensors.lidar) << ",\n";
    f << "    \"imu\": " << sensorJson("", s.sensors.imu) << ",\n";
    f << "    \"wheel\": " << sensorJson("", s.sensors.wheel) << ",\n";
    f << "    \"laserProfiling\": " << sensorJson("", s.sensors.laserProfiling) << ",\n";
    f << "    \"stm\": " << sensorJson("", s.sensors.stm) << ",\n";
    f << "    \"allCalibrated\": " << boolStr(s.sensors.allCalibrated) << "\n";
    f << "  },\n";

    // ndt
    f << "  \"ndt\": {\n";
    f << "    \"gain\": " << s.ndt.gain << ",\n";
    f << "    \"voltage\": " << s.ndt.voltage << ",\n";
    f << "    \"filterId\": " << s.ndt.filterId << "\n";
    f << "  },\n";

    // inspection
    f << "  \"inspection\": {\n";
    f << "    \"surface\": " << quoted(surfaceStr(s.inspection.surface)) << ",\n";
    f << "    \"pipeDiaMm\": " << s.inspection.pipeDiaMm << ",\n";
    f << "    \"probCount\": " << s.inspection.probCount << ",\n";
    f << "    \"rasterLengthMm\": " << s.inspection.rasterLengthMm << ",\n";
    f << "    \"lengthMm\": " << s.inspection.lengthMm << ",\n";
    f << "    \"customLengthMm\": " << s.inspection.customLengthMm << "\n";
    f << "  },\n";

    // runtime
    f << "  \"runtime\": {\n";
    f << "    \"running\": " << boolStr(s.runtime.running) << ",\n";
    f << "    \"frozen\": " << boolStr(s.runtime.frozen) << ",\n";
    f << "    \"mode\": " << quoted(opModeStr(s.runtime.mode)) << "\n";
    f << "  },\n";

    // lastError
    f << "  \"lastError\": {\n";
    f << "    \"source\": " << quoted(s.lastError.source) << ",\n";
    f << "    \"message\": " << quoted(s.lastError.message) << ",\n";
    f << "    \"timestamp\": " << quoted(s.lastError.timestamp) << "\n";
    f << "  }\n";

    f << "}\n";
    std::cout << "\n  Saved to: " << path << "\n";
}

static void printState(const RobotStateData &s)
{
    std::cout << "\n╔══════════════════════════════════════════╗\n";
    std::cout << "║         CURRENT ROBOT STATE              ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";

    std::cout << "\n[Meta]\n";
    std::cout << "  lastUpdatedAt : " << s.lastUpdatedAt << "\n";
    std::cout << "  commandCount  : " << s.commandCount << "\n";

    std::cout << "\n[Connection]\n";
    std::cout << "  state         : " << connStateStr(s.connection.state) << "\n";
    printBool("orinConnected:", s.connection.orinConnected);
    printBool("ndtConnected:", s.connection.ndtConnected);

    std::cout << "\n[RunState]\n";
    std::cout << "  " << runStateStr(s.runState) << "\n";

    std::cout << "\n[Sensors]\n";
    printSensorBlock("Lidar", s.sensors.lidar);
    printSensorBlock("IMU", s.sensors.imu);
    printSensorBlock("Wheel", s.sensors.wheel);
    printSensorBlock("LaserProfiling", s.sensors.laserProfiling);
    printSensorBlock("STM", s.sensors.stm);
    printBool("  allCalibrated:", s.sensors.allCalibrated);

    std::cout << "\n[NDT]\n";
    std::cout << "  gain      : " << s.ndt.gain << "\n";
    std::cout << "  voltage   : " << s.ndt.voltage << " V\n";
    std::cout << "  filterId  : " << s.ndt.filterId << "\n";

    std::cout << "\n[Inspection Config]\n";
    std::cout << "  surface   : " << surfaceStr(s.inspection.surface) << "\n";
    std::cout << "  pipeDia   : " << s.inspection.pipeDiaMm << " mm\n";
    std::cout << "  probCount : " << s.inspection.probCount << "\n";
    std::cout << "  raster    : " << s.inspection.rasterLengthMm << " mm\n";
    std::cout << "  length    : " << s.inspection.lengthMm << " mm\n";
    std::cout << "  customLen : " << s.inspection.customLengthMm << " mm\n";

    std::cout << "\n[Runtime]\n";
    printBool("running:", s.runtime.running);
    printBool("frozen:", s.runtime.frozen);
    std::cout << "  mode      : " << opModeStr(s.runtime.mode) << "\n";

    std::cout << "\n[Last Error]\n";
    std::cout << "  source    : " << (s.lastError.source.empty() ? "(none)" : s.lastError.source) << "\n";
    std::cout << "  message   : " << (s.lastError.message.empty() ? "(none)" : s.lastError.message) << "\n";
    std::cout << "  timestamp : " << (s.lastError.timestamp.empty() ? "(none)" : s.lastError.timestamp) << "\n";
    std::cout << "\n";
}
// ── tiny helpers ──────────────────────────────────────────────

// Extract the value for a key from one JSON object level.
// Handles:  "key": "string"   "key": 1.23   "key": true/false
static std::string jsonValue(const std::string& block, const std::string& key)
{
    std::string needle = "\"" + key + "\"";
    auto pos = block.find(needle);
    if (pos == std::string::npos) return "";

    pos += needle.size();
    while (pos < block.size() && (block[pos] == ' ' || block[pos] == ':')) ++pos;
    if (pos >= block.size()) return "";

    if (block[pos] == '"') {
        ++pos;
        auto end = block.find('"', pos);
        return (end == std::string::npos) ? "" : block.substr(pos, end - pos);
    } else {
        auto end = pos;
        while (end < block.size() && block[end] != ',' && block[end] != '\n'
                                  && block[end] != '}' && block[end] != ' ') ++end;
        return block.substr(pos, end - pos);
    }
}

// Extract a nested  { ... }  block after "key":
static std::string jsonBlock(const std::string& src, const std::string& key)
{
    std::string needle = "\"" + key + "\"";
    auto pos = src.find(needle);
    if (pos == std::string::npos) return "";
    pos = src.find('{', pos + needle.size());
    if (pos == std::string::npos) return "";

    int depth = 0;
    auto start = pos;
    for (auto i = pos; i < src.size(); ++i) {
        if      (src[i] == '{') ++depth;
        else if (src[i] == '}') { if (--depth == 0) return src.substr(start, i - start + 1); }
    }
    return "";
}

static bool     parseBool (const std::string& v) { return v == "true"; }
static float    parseFloat(const std::string& v) { return v.empty() ? 0.0f : std::stof(v); }
static int      parseInt  (const std::string& v) { return v.empty() ? 0    : std::stoi(v); }
static uint64_t parseU64  (const std::string& v) { return v.empty() ? 0ull : std::stoull(v); }

static RobotConnectionState parseConnState(const std::string& v) {
    if (v == "ORIN_CONNECTED")  return RobotConnectionState::ORIN_CONNECTED;
    if (v == "NDT_CONNECTED")   return RobotConnectionState::NDT_CONNECTED;
    if (v == "FULLY_CONNECTED") return RobotConnectionState::FULLY_CONNECTED;
    return RobotConnectionState::DISCONNECTED;
}
static RobotRunState parseRunState(const std::string& v) {
    if (v == "INITIALISED") return RobotRunState::INITIALISED;
    if (v == "RUNNING")     return RobotRunState::RUNNING;
    if (v == "FROZEN")      return RobotRunState::FROZEN;
    if (v == "ERROR")       return RobotRunState::ERROR;
    return RobotRunState::IDLE;
}
static OperationMode parseOpMode(const std::string& v) {
    if (v == "AUTOMATIC") return OperationMode::AUTOMATIC;
    if (v == "SEMI_AUTO") return OperationMode::SEMI_AUTO;
    return OperationMode::MANUAL;
}
static InspectionSurface parseSurface(const std::string& v) {
    if (v == "CURVED") return InspectionSurface::CURVED;
    if (v == "PIPE")   return InspectionSurface::PIPE;
    if (v == "CUSTOM") return InspectionSurface::CUSTOM;
    return InspectionSurface::FLAT;
}

static void parseSensorFlags(const std::string& block,
                              RobotStateData::SensorFlags& f)
{
    f.initialised = parseBool(jsonValue(block, "initialised"));
    f.bypassed    = parseBool(jsonValue(block, "bypassed"));
    f.calibrated  = parseBool(jsonValue(block, "calibrated"));
}

// ── loadJSON ──────────────────────────────────────────────────

static bool loadJSON(RobotStateData& s, const std::string& path)
{
    std::ifstream f(path);
    if (!f) {
        std::cerr << "[loadJSON] Cannot open: " << path << "\n";
        return false;
    }

    std::string src((std::istreambuf_iterator<char>(f)),
                     std::istreambuf_iterator<char>());

    // meta
    s.lastUpdatedAt = jsonValue(src, "lastUpdatedAt");
    s.commandCount  = parseU64(jsonValue(src, "commandCount"));

    // connection
    auto connBlock          = jsonBlock(src, "connection");
    s.connection.state         = parseConnState(jsonValue(connBlock, "state"));
    s.connection.orinConnected = parseBool(jsonValue(connBlock, "orinConnected"));
    s.connection.ndtConnected  = parseBool(jsonValue(connBlock, "ndtConnected"));

    // runState
    s.runState = parseRunState(jsonValue(src, "runState"));

    // sensors
    auto sensorsBlock = jsonBlock(src, "sensors");
    parseSensorFlags(jsonBlock(sensorsBlock, "lidar"),          s.sensors.lidar);
    parseSensorFlags(jsonBlock(sensorsBlock, "imu"),            s.sensors.imu);
    parseSensorFlags(jsonBlock(sensorsBlock, "wheel"),          s.sensors.wheel);
    parseSensorFlags(jsonBlock(sensorsBlock, "laserProfiling"), s.sensors.laserProfiling);
    parseSensorFlags(jsonBlock(sensorsBlock, "stm"),            s.sensors.stm);
    s.sensors.allCalibrated = parseBool(jsonValue(sensorsBlock, "allCalibrated"));

    // ndt
    auto ndtBlock    = jsonBlock(src, "ndt");
    s.ndt.gain       = parseFloat(jsonValue(ndtBlock, "gain"));
    s.ndt.voltage    = parseFloat(jsonValue(ndtBlock, "voltage"));
    s.ndt.filterId   = parseInt  (jsonValue(ndtBlock, "filterId"));

    // inspection
    auto inspBlock              = jsonBlock(src, "inspection");
    s.inspection.surface        = parseSurface(jsonValue(inspBlock, "surface"));
    s.inspection.pipeDiaMm      = parseFloat(jsonValue(inspBlock, "pipeDiaMm"));
    s.inspection.probCount      = parseInt  (jsonValue(inspBlock, "probCount"));
    s.inspection.rasterLengthMm = parseFloat(jsonValue(inspBlock, "rasterLengthMm"));
    s.inspection.lengthMm       = parseFloat(jsonValue(inspBlock, "lengthMm"));
    s.inspection.customLengthMm = parseFloat(jsonValue(inspBlock, "customLengthMm"));

    // runtime
    auto rtBlock      = jsonBlock(src, "runtime");
    s.runtime.running = parseBool  (jsonValue(rtBlock, "running"));
    s.runtime.frozen  = parseBool  (jsonValue(rtBlock, "frozen"));
    s.runtime.mode    = parseOpMode(jsonValue(rtBlock, "mode"));

    // lastError
    auto errBlock         = jsonBlock(src, "lastError");
    s.lastError.source    = jsonValue(errBlock, "source");
    s.lastError.message   = jsonValue(errBlock, "message");
    s.lastError.timestamp = jsonValue(errBlock, "timestamp");

    std::cout << "[loadJSON] Loaded from: " << path << "\n";
    return true;
}

// int main()
// {

//     // Load the settings from a JSON file (if it exists)
//     RobotStateData state;
//     loadJSON(state, "robot_state_test.json");
//     printState(state);

//     state.lastUpdatedAt = nowISO();

//     state.connection.state = RobotConnectionState::FULLY_CONNECTED;
//     state.connection.orinConnected = true;
//     state.connection.ndtConnected = true;
//     state.runState = RobotRunState::INITIALISED;
//     state.sensors.lidar.initialised = true;
//     state.sensors.lidar.calibrated = true;
//     state.sensors.imu.initialised = true;
//     state.sensors.imu.calibrated = true;
//     state.sensors.wheel.initialised = true;
//     state.sensors.wheel.calibrated = true;
//     state.sensors.laserProfiling.initialised = true;
//     state.sensors.laserProfiling.calibrated = true;
//     state.sensors.stm.initialised = true;
//     state.sensors.stm.calibrated = true;
//     state.sensors.allCalibrated = true;
//     state.ndt.gain = 1.5f;
//     state.ndt.voltage = 12.0f;
//     state.ndt.filterId = 2;
//     state.inspection.surface = InspectionSurface::PIPE;
//     const std::string outFile = "robot_state_test.json";
//     printState(state);
//     saveJSON(state, outFile);
// }