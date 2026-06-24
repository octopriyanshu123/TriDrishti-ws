#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <cstdint>


enum class RobotConnectionState {
    DISCONNECTED, ORIN_CONNECTED, NDT_CONNECTED, FULLY_CONNECTED
};
enum class RobotRunState {
    IDLE, INITIALISED, RUNNING, FROZEN, ERROR
};
enum class OperationMode {
    MANUAL, AUTOMATIC, SEMI_AUTO
};
enum class InspectionSurface {
    FLAT, CURVED, PIPE, CUSTOM
};


template<size_t N>
struct FixedStr {
    char buf[N]{};

    FixedStr() = default;

    FixedStr(const std::string& s) {
        std::strncpy(buf, s.c_str(), N - 1);
        buf[N - 1] = '\0';
    }

    FixedStr& operator=(const std::string& s) {
        std::memset(buf, 0, N);
        std::strncpy(buf, s.c_str(), N - 1);
        return *this;
    }

    std::string str() const { return std::string(buf); }

    bool operator==(const std::string& s) const { return str() == s; }
};


struct RobotStateData {

    FixedStr<64>  lastCommand;
    FixedStr<32>  lastUpdatedAt;
    uint64_t      commandCount{0};

    struct Connection {
        RobotConnectionState state{RobotConnectionState::DISCONNECTED};
        bool orinConnected{false};
        bool ndtConnected{false};
    } connection;

    RobotRunState runState{RobotRunState::IDLE};

    struct SensorFlags {
        bool initialised{false};
        bool bypassed{false};
        bool calibrated{false};
    };
    struct Sensors {
        SensorFlags lidar;
        SensorFlags imu;
        SensorFlags wheel;
        SensorFlags laserProfiling;
        SensorFlags stm;
        bool allCalibrated{false};
    } sensors;

    struct NDT {
        float gain{1.0f};
        float voltage{0.0f};
        int   filterId{0};
    } ndt;

    struct Inspection {
        InspectionSurface surface{InspectionSurface::FLAT};
        float pipeDiaMm{0.0f};
        int   probCount{1};
        float rasterLengthMm{0.0f};
        float lengthMm{0.0f};
        float customLengthMm{0.0f};
    } inspection;

    struct InspectionRuntime {
        bool          running{false};
        bool          frozen{false};
        OperationMode mode{OperationMode::MANUAL};
    } runtime;

    struct LastError {
        FixedStr<32> source;
        FixedStr<128> message;
        FixedStr<32>  timestamp;
    } lastError;
};

static const char* toString(RobotConnectionState s) {
    switch (s) {
        case RobotConnectionState::DISCONNECTED:    return "DISCONNECTED";
        case RobotConnectionState::ORIN_CONNECTED:  return "ORIN_CONNECTED";
        case RobotConnectionState::NDT_CONNECTED:   return "NDT_CONNECTED";
        case RobotConnectionState::FULLY_CONNECTED: return "FULLY_CONNECTED";
        default:                                    return "UNKNOWN";
    }
}
static const char* toString(RobotRunState s) {
    switch (s) {
        case RobotRunState::IDLE:        return "IDLE";
        case RobotRunState::INITIALISED: return "INITIALISED";
        case RobotRunState::RUNNING:     return "RUNNING";
        case RobotRunState::FROZEN:      return "FROZEN";
        case RobotRunState::ERROR:       return "ERROR";
        default:                         return "UNKNOWN";
    }
}
static const char* toString(OperationMode m) {
    switch (m) {
        case OperationMode::MANUAL:    return "MANUAL";
        case OperationMode::AUTOMATIC: return "AUTOMATIC";
        case OperationMode::SEMI_AUTO: return "SEMI_AUTO";
        default:                       return "UNKNOWN";
    }
}
static const char* toString(InspectionSurface s) {
    switch (s) {
        case InspectionSurface::FLAT:   return "FLAT";
        case InspectionSurface::CURVED: return "CURVED";
        case InspectionSurface::PIPE:   return "PIPE";
        case InspectionSurface::CUSTOM: return "CUSTOM";
        default:                        return "UNKNOWN";
    }
}
static const char* boolStr(bool v) { return v ? "true" : "false"; }


class StateManager {
public:
    explicit StateManager(const std::string& filename)
        : filename_(filename) {}


    RobotStateData load() {
        std::ifstream file(filename_, std::ios::binary);
        if (!file.is_open()) {
            return RobotStateData{};
        }

        RobotStateData rs;
        file.read(reinterpret_cast<char*>(&rs), sizeof(RobotStateData));
        if (!file) {
            throw std::runtime_error(
                "StateManager: failed to read from " + filename_);
        }
        return rs;
    }

    void save(const RobotStateData& rs) {
        std::ofstream file(filename_, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error(
                "StateManager: cannot open file for writing: " + filename_);
        }

        file.write(reinterpret_cast<const char*>(&rs), sizeof(RobotStateData));
        if (!file) {
            throw std::runtime_error(
                "StateManager: failed to write to " + filename_);
        }
    }

    void printStatus(const RobotStateData& rs) const {
        const char* B = "\033[1m";   // bold
        const char* R = "\033[0m";   // reset
        const char* C = "\033[36m";  // cyan  (section headers)
        const char* Y = "\033[33m";  // yellow
        const char* E = "\033[31m";  // red   (errors / estop)
        const char* G = "\033[32m";  // green

        std::cout << B << "┌─────────────────────────────────────────┐\n";
        std::cout <<      "│            Robot State Status            │\n";
        std::cout <<      "└─────────────────────────────────────────┘\n" << R;

        // ── General ──────────────────────────────
        std::cout << C << B << "  [General]\n" << R;
        std::cout << "    lastCommand   : " << rs.lastCommand.str()   << "\n";
        std::cout << "    lastUpdatedAt : " << rs.lastUpdatedAt.str() << "\n";
        std::cout << "    commandCount  : " << rs.commandCount        << "\n";

        // ── Connection ───────────────────────────
        std::cout << C << B << "  [Connection]\n" << R;
        std::cout << "    state         : " << Y << toString(rs.connection.state) << R << "\n";
        std::cout << "    orinConnected : " << boolStr(rs.connection.orinConnected) << "\n";
        std::cout << "    ndtConnected  : " << boolStr(rs.connection.ndtConnected)  << "\n";

        // ── Run State ────────────────────────────
        std::cout << C << B << "  [Run State]\n" << R;
        std::cout << "    runState      : " << Y << toString(rs.runState) << R << "\n";

        // ── Sensors ──────────────────────────────
        std::cout << C << B << "  [Sensors]\n" << R;
        auto printSensor = [&](const char* name, const RobotStateData::SensorFlags& f) {
            std::cout << "    " << B << name << R
                      << "  init=" << boolStr(f.initialised)
                      << "  bypass=" << boolStr(f.bypassed)
                      << "  calib=" << (f.calibrated ? G : E)
                      << boolStr(f.calibrated) << R << "\n";
        };
        printSensor("lidar        ", rs.sensors.lidar);
        printSensor("imu          ", rs.sensors.imu);
        printSensor("wheel        ", rs.sensors.wheel);
        printSensor("laserProfiling", rs.sensors.laserProfiling);
        printSensor("stm          ", rs.sensors.stm);
        std::cout << "    allCalibrated : " << boolStr(rs.sensors.allCalibrated) << "\n";

        // ── NDT ──────────────────────────────────
        std::cout << C << B << "  [NDT]\n" << R;
        std::cout << "    gain     : " << rs.ndt.gain     << "\n";
        std::cout << "    voltage  : " << rs.ndt.voltage  << "\n";
        std::cout << "    filterId : " << rs.ndt.filterId << "\n";

        // ── Inspection ───────────────────────────
        std::cout << C << B << "  [Inspection]\n" << R;
        std::cout << "    surface        : " << toString(rs.inspection.surface)       << "\n";
        std::cout << "    pipeDiaMm      : " << rs.inspection.pipeDiaMm              << " mm\n";
        std::cout << "    probCount      : " << rs.inspection.probCount               << "\n";
        std::cout << "    rasterLengthMm : " << rs.inspection.rasterLengthMm         << " mm\n";
        std::cout << "    lengthMm       : " << rs.inspection.lengthMm               << " mm\n";
        std::cout << "    customLengthMm : " << rs.inspection.customLengthMm         << " mm\n";

        // ── Runtime ──────────────────────────────
        std::cout << C << B << "  [Runtime]\n" << R;
        std::cout << "    running : " << boolStr(rs.runtime.running) << "\n";
        std::cout << "    frozen  : " << boolStr(rs.runtime.frozen)  << "\n";
        std::cout << "    mode    : " << toString(rs.runtime.mode)   << "\n";

        // ── Last Error ───────────────────────────
        std::cout << C << B << "  [Last Error]\n" << R;
        const bool hasError = rs.lastError.message.buf[0] != '\0';
        if (hasError) {
            std::cout << E;
        }
        std::cout << "    source    : " << rs.lastError.source.str()    << "\n";
        std::cout << "    message   : " << rs.lastError.message.str()   << "\n";
        std::cout << "    timestamp : " << rs.lastError.timestamp.str() << R << "\n";

        std::cout << B << "──────────────────────────────────────────\n" << R;
    }

private:
    std::string filename_;
};


int main() {
    StateManager sm("robot_state_ka.bin");

    RobotStateData rs = sm.load();

    rs.lastCommand   = "INIT_SEQUENCE";
    rs.lastUpdatedAt = "2026-06-:00:00";
    rs.commandCount  = 1;

    rs.connection.state        = RobotConnectionState::ORIN_CONNECTED;
    rs.connection.orinConnected = true;
    rs.connection.ndtConnected  = false;

    rs.runState = RobotRunState::INITIALISED;

    rs.sensors.lidar.initialised = true;
    rs.sensors.lidar.bypassed    = false;
    rs.sensors.lidar.calibrated  = true;
    rs.sensors.imu.initialised   = true;
    rs.sensors.imu.bypassed      = false;
    rs.sensors.imu.calibrated    = false;

    rs.ndt.gain    = 1.5f;
    rs.ndt.voltage = 12.0f;

    rs.inspection.surface   = InspectionSurface::PIPE;
    rs.inspection.pipeDiaMm = 150.0f;
    rs.inspection.probCount = 2;

    rs.runtime.mode    = OperationMode::SEMI_AUTO;
    rs.runtime.running = false;

    sm.save(rs);

    RobotStateData loaded = sm.load();
    sm.printStatus(loaded);

    return 0;
}