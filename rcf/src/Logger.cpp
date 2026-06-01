#include "rcf/Logger.hpp"

#include <iostream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <iomanip>

namespace rcf {

namespace color {
    static constexpr const char* RESET   = "\033[0m";
    static constexpr const char* RED     = "\033[31m";
    static constexpr const char* GREEN   = "\033[32m";
    static constexpr const char* YELLOW  = "\033[33m";
    static constexpr const char* MAGENTA = "\033[35m";
    static constexpr const char* CYAN    = "\033[36m";
    static constexpr const char* WHITE   = "\033[37m";
    static constexpr const char* BOLD    = "\033[1m";
}

// ─── PIMPL body ───────────────────────────────────────────────────────────────
struct Logger::Impl {
    std::mutex  mtx;
    Level       level{ Level::DEBUG };
};

// ─── Singleton ────────────────────────────────────────────────────────────────
Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::Logger() : impl_(new Impl{}) {}

void Logger::setLevel(Level l) {
    std::lock_guard<std::mutex> lk(impl_->mtx);
    impl_->level = l;
}

void Logger::log(Level lvl, const std::string& tag, const std::string& msg) {
    {
        std::lock_guard<std::mutex> lk(impl_->mtx);
        if (lvl < impl_->level) return;
    }

    auto now = std::chrono::system_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                   now.time_since_epoch()) % 1000;
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::tm tm_b{};
    localtime_r(&t, &tm_b);

    std::ostringstream ts;
    ts << std::put_time(&tm_b, "%H:%M:%S") << '.'
       << std::setw(3) << std::setfill('0') << ms.count();

    const char *ls, *lc;
    switch(lvl){
        case Level::DEBUG: ls="DBG"; lc=color::CYAN;   break;
        case Level::INFO:  ls="INF"; lc=color::GREEN;  break;
        case Level::WARN:  ls="WRN"; lc=color::YELLOW; break;
        case Level::ERROR: ls="ERR"; lc=color::RED;    break;
        default:           ls="???"; lc=color::WHITE;  break;
    }

    std::lock_guard<std::mutex> lk(impl_->mtx);
    std::cout << color::WHITE  << "[" << ts.str() << "] "
              << lc << color::BOLD << "[" << ls << "] "
              << color::MAGENTA    << "[" << tag << "] "
              << color::RESET      << msg << "\n";
}

} // namespace rcf
