/*
Singalto Logger calss Use in Main Robot
It have 3 configration Log Level File Path and DetailFlag Log

@1 Level Logger from 1 to 4
1: Debug
2: Info
3: Warning
4: Error
If I have set the Level Logger to 2, I will see the Info, Warning and Error Log but not Debug Log

@ 2 Auto find the username and log folder int home directory

@3 Details log will show the file name, line number and function name where the log is called


Guide to use Logger:
#include "Logger.hpp"

int main()
{
    // Configure once at startup: show INFO and above, also write to a file
    Logger::getInstance().configure(Logger::LogLevel::INFO, "robot.log", false);
    Logger &log = Logger::getInstance();
    LOG_DEBUG  ("Main",            "Starting up…");
    LOG_INFO   ("Main",            "Robot initialised");
    LOG_WARNING("MotorController", "Left encoder jitter detected");
    LOG_ERROR  ("PowerModule",     "Battery voltage critical");

    return 0;
}


class Singleton
{
public:
// Single Global Access Point A static function returns the only instance.
    static Singleton& getInstance()
    {
        static Singleton instance;
        return instance;
    }

private:
// Private Constructor Prevents other code from creating instances.
    Singleton() = default;

    // Delete Copy Constructor Copy Assignment Operator Move Constructor Move Assignment Operator

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};

*/

#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <pwd.h>    
#include <unistd.h> 

#include <filesystem>
#include <cstdlib>   

// helpers add the /home/<username>/log/ prefix to the filename and create the directory if needed
static std::string resolveLogPath(const std::string &filename)
{
    const char *home = std::getenv("HOME");
    if (!home)
    {
        home = getpwuid(getuid())->pw_dir; // needs <pwd.h> + <unistd.h>
    }
    std::filesystem::path logDir = std::filesystem::path(home) / "log";
    std::filesystem::create_directories(logDir);
    return (logDir / filename).string();
}
// configuration method: set minimum log level and optionally a file to write to
void Logger::configure(LogLevel minLevel, const std::string &filename, bool Detail)
{
    std::lock_guard<std::mutex> guard(mutex_);
    minLevel_ = minLevel;
    detail_ = Detail;
    std::cout << "[Logger] Detail to: " << detail_ << "\n";

    
    if (!filename.empty())
    {
        filePath_ = resolveLogPath(filename); // ← resolve the full path here

        fileStream_.open(filePath_, std::ios::app);
        if (!fileStream_.is_open())
            std::cerr << "[Logger] WARNING: could not open log file: " << filePath_ << "\n";
        else
            std::cout << "[Logger] Logging to: " << filePath_ << "\n";
    }
}

Logger::~Logger()
{
    if (fileStream_.is_open())
        fileStream_.close();
}

const char *Logger::levelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO ";
    case LogLevel::WARNING:
        return "WARN ";
    case LogLevel::ERROR:
        return "ERROR";
    default:
        return "?????";
    }
}


void Logger::debug(const std::string &c, const std::string &m, const char *file, int line, const char *func)
{
    log(LogLevel::DEBUG, c, m, file, line, func);
}

void Logger::info(const std::string &c, const std::string &m, const char *file, int line, const char *func)
{
    log(LogLevel::INFO, c, m, file, line, func);
}

void Logger::warning(const std::string &c, const std::string &m, const char *file, int line, const char *func)
{
    log(LogLevel::WARNING, c, m, file, line, func);
}

void Logger::error(const std::string &c, const std::string &m, const char *file, int line, const char *func)
{
    log(LogLevel::ERROR, c, m, file, line, func);
}


void Logger::log(LogLevel level, const std::string &component, const std::string &message,
                 const char *file, int line, const char *func)
{
    if (level < minLevel_)
        return;

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char timeBuf[20];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));

    std::string shortFile = std::filesystem::path(file).filename().string();

    // Format
        std::ostringstream line_stream;

    if(detail_)
    {
        line_stream << "[" << timeBuf << "] "
                    << "[" << levelToString(level) << "] "
                    << "[" << component << "] "
                    << message
                    << "  (" << shortFile << ":" << line << " @ " << func << ")\n";
    }
    else
    {
        line_stream << "[" << timeBuf << "] "
                    << "[" << levelToString(level) << "] "
                    << "[" << component << "] "
                    << message
                    << "\n";
    }

    std::lock_guard<std::mutex> guard(mutex_);
    std::cout << line_stream.str();
    if (fileStream_.is_open())
        fileStream_ << line_stream.str();
}
