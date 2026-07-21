#include "RobotState.hpp"

class StateManager
{
private:
    /* data */
public:
    StateManager(/* args */);
    ~StateManager();
    static void printSensorBlock(const std::string &name,
                                 const RobotStateData::SensorFlags &f);
    static void printState(const RobotStateData &s);
    static void printBool(const std::string &label, bool v);
    static std::string nowISO();
    static std::string boolStr(bool v);
    static std::string quoted(const std::string &s);
    static std::string connStateStr(RobotConnectionState s);
    static std::string runStateStr(RobotRunState s);

    static RobotConnectionState parseConnState(const std::string &v);

    static RobotRunState parseRunState(const std::string &v);
    static OperationMode parseOpMode(const std::string &v);

    static InspectionSurface parseSurface(const std::string &v);

    static void parseSensorFlags(const std::string &block, RobotStateData::SensorFlags &f);

    static bool loadJSON(RobotStateData &s, const std::string &path);
    static void saveJSON(const RobotStateData &s, const std::string &path);

};
