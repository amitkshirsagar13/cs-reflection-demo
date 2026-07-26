#include "Logger.h"

namespace UserProfileModel {

// ANSI colour codes
namespace Colour {
    constexpr const char* RESET  = "\033[0m";
    constexpr const char* GREY   = "\033[90m";
    constexpr const char* CYAN   = "\033[36m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* RED    = "\033[31m";
}

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::~Logger() {
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
}

void Logger::configure(LogLevel minLevel, bool logToFile,
                        const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel   = minLevel;
    m_logToFile  = logToFile;
    if (logToFile && !filePath.empty()) {
        m_fileStream.open(filePath, std::ios::app);
    }
}

void Logger::debug(const std::string& msg, const std::string& module) {
    log(LogLevel::DEBUG, msg, module);
}
void Logger::info(const std::string& msg, const std::string& module) {
    log(LogLevel::INFO, msg, module);
}
void Logger::warn(const std::string& msg, const std::string& module) {
    log(LogLevel::WARN, msg, module);
}
void Logger::error(const std::string& msg, const std::string& module) {
    log(LogLevel::ERROR, msg, module);
}

void Logger::log(LogLevel level, const std::string& msg,
                 const std::string& module) {
    if (level < m_minLevel) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    std::string tag   = levelTag(level);
    std::string ts    = timestamp();
    std::string modPart = module.empty() ? "" : "[" + module + "] ";

    // Pick colour
    const char* colour = Colour::RESET;
    switch (level) {
        case LogLevel::DEBUG: colour = Colour::GREY;   break;
        case LogLevel::INFO:  colour = Colour::CYAN;   break;
        case LogLevel::WARN:  colour = Colour::YELLOW; break;
        case LogLevel::ERROR: colour = Colour::RED;    break;
    }

    // Console (coloured)
    std::cout << colour
              << ts << " " << tag << " " << modPart << msg
              << Colour::RESET << "\n";

    // File (plain)
    if (m_logToFile && m_fileStream.is_open()) {
        m_fileStream << ts << " " << tag << " " << modPart << msg << "\n";
        m_fileStream.flush();
    }
}

std::string Logger::levelTag(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "[DEBUG]";
        case LogLevel::INFO:  return "[INFO ]";
        case LogLevel::WARN:  return "[WARN ]";
        case LogLevel::ERROR: return "[ERROR]";
    }
    return "[?????]";
}

std::string Logger::timestamp() {
    auto now  = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms   = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) % 1000;
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

} // namespace UserProfileModel