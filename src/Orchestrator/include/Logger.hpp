#pragma once

#include <string>
#include <fstream>
#include <mutex>

class Logger
{
public:
    enum class LogLevel
    {
        DEBUG = 1,
        INFO = 2,
        WARNING = 3,
        ERROR = 4
    };
    static Logger &getInstance()
    {
        static Logger instance;
        return instance;
    }
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;

    void configure(LogLevel minLevel, const std::string &filePath = "" , bool Detail = false    );

    void debug(const std::string &component, const std::string &message,
               const char *file, int line, const char *func);

    void info(const std::string &component, const std::string &message,
              const char *file, int line, const char *func);

    void warning(const std::string &component, const std::string &message,
                 const char *file, int line, const char *func);

    void error(const std::string &component, const std::string &message,
               const char *file, int line, const char *func);

private:
    void log(LogLevel level, const std::string &component, const std::string &message,
             const char *file, int line, const char *func);

private:
    Logger() = default;
    ~Logger();

    static const char *levelToString(LogLevel level);

    LogLevel minLevel_{LogLevel::DEBUG};
    std::string filePath_;
    std::ofstream fileStream_;
    bool detail_{false};
    std::mutex mutex_;
};

// Must be macros (not functions) so __FILE__ / __LINE__ / __func__
#define LOG_DEBUG(component, message) \
    Logger::getInstance().debug(component, message, __FILE__, __LINE__, __func__)

#define LOG_INFO(component, message) \
    Logger::getInstance().info(component, message, __FILE__, __LINE__, __func__)

#define LOG_WARNING(component, message) \
    Logger::getInstance().warning(component, message, __FILE__, __LINE__, __func__)

#define LOG_ERROR(component, message) \
    Logger::getInstance().error(component, message, __FILE__, __LINE__, __func__)