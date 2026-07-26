#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace UserProfileModel {

enum class LogLevel {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3
};

class Logger {
public:
    static Logger& instance();

    void configure(LogLevel minLevel,
                   bool     logToFile,
                   const std::string& filePath = "");

    void debug(const std::string& msg, const std::string& module = "");
    void info (const std::string& msg, const std::string& module = "");
    void warn (const std::string& msg, const std::string& module = "");
    void error(const std::string& msg, const std::string& module = "");

    LogLevel minLevel() const { return m_minLevel; }

private:
    Logger() = default;
    ~Logger();

    void log(LogLevel level, const std::string& msg, const std::string& module);
    static std::string levelTag(LogLevel level);
    static std::string timestamp();

    LogLevel          m_minLevel{LogLevel::INFO};
    bool              m_logToFile{false};
    std::ofstream     m_fileStream;
    std::mutex        m_mutex;
};

// Convenience macros
#define LOG_DEBUG(msg) UserProfileModel::Logger::instance().debug(msg, __func__)
#define LOG_INFO(msg)  UserProfileModel::Logger::instance().info (msg, __func__)
#define LOG_WARN(msg)  UserProfileModel::Logger::instance().warn (msg, __func__)
#define LOG_ERROR(msg) UserProfileModel::Logger::instance().error(msg, __func__)

} // namespace UserProfileModel