#include "Config.h"
#include "Logger.h"
#include "UserStore.h"
#include "CsBridgeInvoker.h"
#include "Menus.h"

#include <iostream>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────
//  Bootstrap: configure logger from AppConfig
// ─────────────────────────────────────────────────────────────────────────
static void configureLogger(const UserProfileModel::AppConfig& cfg) {
    using namespace UserProfileModel;

    LogLevel level = LogLevel::INFO;
    std::string lvl = cfg.logLevel;
    // case-insensitive compare
    for (auto& c : lvl) c = static_cast<char>(std::toupper(c));
    if      (lvl == "DEBUG") level = LogLevel::DEBUG;
    else if (lvl == "WARN")  level = LogLevel::WARN;
    else if (lvl == "ERROR") level = LogLevel::ERROR;

    Logger::instance().configure(level, cfg.logToFile, cfg.logFilePath);
}

// ─────────────────────────────────────────────────────────────────────────
//  Register default C# scripts in the bridge
// ─────────────────────────────────────────────────────────────────────────
static void registerDefaultScripts(App::CsBridgeInvoker& bridge,
                                    const fs::path& exeDir,
                                    const UserProfileModel::AppConfig& cfg) {
    using App::CsScriptDescriptor;

    // Resolve the C# runner DLL relative to exe dir unless absolute
    fs::path runnerPath = cfg.csRunnerPath;
    if (runnerPath.is_relative())
        runnerPath = exeDir / runnerPath;

    // ── Profile Loader ────────────────────────────────────────────────────
    App::CsScriptDescriptor profileLoader;
    profileLoader.name        = "profile_loader";
    profileLoader.dllPath     = runnerPath.string();
    profileLoader.typeName    = "CsRunner.ProfileLoader";
    profileLoader.description = "Loads user profiles from JSON/YAML via C# reflection";
    bridge.registerScript(profileLoader);

    // ── Future scripts can be registered here ─────────────────────────────
    // bridge.registerScript({
    //     .name        = "report_generator",
    //     .dllPath     = (exeDir / "csbridge/ReportGenerator.dll").string(),
    //     .typeName    = "CsRunner.ReportGenerator",
    //     .description = "Generates PDF reports from profile data"
    // });
}

// ─────────────────────────────────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────────────────────────────────
int main(int /*argc*/, char* argv[]) {
    // Determine directory where the executable lives
    fs::path exePath = fs::canonical(fs::path(argv[0]));
    fs::path exeDir  = exePath.parent_path();

    // Load configuration
    auto config = UserProfileModel::ConfigReader::load(exeDir.string());

    // Configure logger
    configureLogger(config);

    LOG_INFO("=== " + config.appName + " v" + config.version + " starting ===");
    LOG_DEBUG("exe dir: " + exeDir.string());

    // Create store and bridge
    App::UserStore       store;
    App::CsBridgeInvoker bridge("dotnet");  // dotnet must be on PATH

    registerDefaultScripts(bridge, exeDir, config);

    // Launch interactive menus
    try {
        App::Menus::runMainMenu(store, bridge, config, exeDir);
    } catch (std::exception& ex) {
        LOG_ERROR(std::string("Unhandled exception: ") + ex.what());
        std::cerr << "\033[31mFatal error: " << ex.what() << "\033[0m\n";
        return 1;
    }

    return 0;
}