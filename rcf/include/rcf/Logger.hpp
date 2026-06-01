#pragma once
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  Logger  —  declaration only.  All implementation is in Logger.cpp.
//  Clients see the macros and the Level enum; nothing else leaks.
// ─────────────────────────────────────────────────────────────────────────────

namespace rcf {

class Logger {
public:
    enum class Level { DEBUG=0, INFO=1, WARN=2, ERROR=3 };

    static Logger& instance();          // defined in Logger.cpp
    void setLevel(Level l);
    void log(Level lvl, const std::string& tag, const std::string& msg);

private:
    Logger();                           // constructed inside Logger.cpp
    struct Impl;                        // PIMPL — hides mutex / state
    Impl* impl_;                        // raw ptr: Logger is a singleton,
                                        // never deleted, so no leak
};

} // namespace rcf

// Convenience macros — zero overhead when level is filtered
#define RIPC_DEBUG(tag,msg) rcf::Logger::instance().log(rcf::Logger::Level::DEBUG,tag,msg)
#define RIPC_INFO(tag,msg)  rcf::Logger::instance().log(rcf::Logger::Level::INFO, tag,msg)
#define RIPC_WARN(tag,msg)  rcf::Logger::instance().log(rcf::Logger::Level::WARN, tag,msg)
#define RIPC_ERROR(tag,msg) rcf::Logger::instance().log(rcf::Logger::Level::ERROR,tag,msg)
