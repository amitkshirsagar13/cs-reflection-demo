#include "Config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <iostream>

namespace UserProfileModel {

std::string ConfigReader::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return {};
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool ConfigReader::parseBool(const std::string& s) {
    std::string v = s;
    std::transform(v.begin(), v.end(), v.begin(), ::tolower);
    return (v == "true" || v == "yes" || v == "1");
}

AppConfig ConfigReader::load(const std::string& dir) {
    AppConfig cfg;

    // Try conf.yml then conf.yaml
    std::vector<std::string> candidates = {
        dir + "/conf.yml",
        dir + "/conf.yaml"
    };

    std::ifstream file;
    std::string   usedPath;
    for (auto& p : candidates) {
        file.open(p);
        if (file.is_open()) { usedPath = p; break; }
    }

    if (!file.is_open()) {
        std::cout << "\033[33m[Config] conf.yml not found in '"
                  << dir << "', using defaults.\033[0m\n";
        return cfg;
    }

    std::cout << "\033[36m[Config] Loading " << usedPath << "\033[0m\n";

    std::string currentSection;
    std::string line;

    while (std::getline(file, line)) {
        // Strip comments
        auto commentPos = line.find('#');
        if (commentPos != std::string::npos)
            line = line.substr(0, commentPos);

        line = trim(line);
        if (line.empty()) continue;

        // Section header  [section]
        if (line.front() == '[' && line.back() == ']') {
            currentSection = trim(line.substr(1, line.size() - 2));
            continue;
        }

        // key: value
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;

        std::string key   = trim(line.substr(0, colonPos));
        std::string value = trim(line.substr(colonPos + 1));

        // Strip surrounding quotes from value
        if (value.size() >= 2 &&
            ((value.front() == '"' && value.back() == '"') ||
             (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }

        if (currentSection == "logging") {
            if      (key == "level")     cfg.logLevel    = value;
            else if (key == "to_file")   cfg.logToFile   = parseBool(value);
            else if (key == "file_path") cfg.logFilePath = value;
        } else if (currentSection == "app") {
            if      (key == "name")    cfg.appName  = value;
            else if (key == "version") cfg.version  = value;
        } else if (currentSection == "csbridge") {
            if (key == "runner_path")  cfg.csRunnerPath = value;
        } else {
            cfg.extras[currentSection + "." + key] = value;
        }
    }

    return cfg;
}

} // namespace UserProfileModel