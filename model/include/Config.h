#pragma once
#include <string>
#include <map>
#include "Logger.h"

namespace UserProfileModel {

struct AppConfig {
    // [logging]
    std::string logLevel{"INFO"};
    bool        logToFile{false};
    std::string logFilePath{"app.log"};

    // [app]
    std::string appName{"UserProfileApp"};
    std::string version{"1.0.0"};

    // [csbridge]
    std::string csRunnerPath{"./csbridge/CsRunner.dll"};

    // arbitrary extra keys
    std::map<std::string, std::string> extras;
};

class ConfigReader {
public:
    // Load conf.yml (or conf.yaml) from the given directory.
    // Falls back to defaults if file absent.
    static AppConfig load(const std::string& dir = ".");

private:
    static std::string trim(const std::string& s);
    static bool        parseBool(const std::string& s);
};

} // namespace UserProfileModel