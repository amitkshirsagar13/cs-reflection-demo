#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <stdexcept>
#include "Logger.h"
#include "UserProfile.h"

namespace App {

// ─────────────────────────────────────────────────────────────────────────
//  CsScriptDescriptor
//  Describes a registered C# script that the bridge can invoke.
// ─────────────────────────────────────────────────────────────────────────
struct CsScriptDescriptor {
    std::string name;          // symbolic name used to look it up
    std::string dllPath;       // path to the compiled .dll
    std::string typeName;      // fully qualified type name inside the dll
    std::string description;   // human readable
};

// ─────────────────────────────────────────────────────────────────────────
//  InvocationResult  – returned from every bridge call
// ─────────────────────────────────────────────────────────────────────────
struct InvocationResult {
    bool                                       success{false};
    std::string                                errorMessage;
    std::vector<UserProfileModel::UserProfile> profiles; // populated by loaders
    int                                        exitCode{-1};
};

// ─────────────────────────────────────────────────────────────────────────
//  CsBridgeInvoker
//  Utility class that manages the registry of known C# scripts and knows
//  how to invoke them through the dotnet runtime host (dotnet exec).
//
//  Registration is done at startup; new C# scripts are added by calling
//  registerScript() before invoking them by symbolic name.
// ─────────────────────────────────────────────────────────────────────────
class CsBridgeInvoker {
public:
    explicit CsBridgeInvoker(std::string dotnetExe = "dotnet");

    // ── Registry API ─────────────────────────────────────────────────────
    void registerScript(CsScriptDescriptor descriptor);
    bool isRegistered(const std::string& name) const;
    const CsScriptDescriptor& getDescriptor(const std::string& name) const;
    std::vector<std::string> registeredNames() const;

    // ── Invocation API ───────────────────────────────────────────────────
    // Invoke a registered script by symbolic name.
    // args: extra CLI arguments passed to the script.
    InvocationResult invoke(const std::string& scriptName,
                            const std::vector<std::string>& args = {}) const;

    // Convenience: invoke the "profile_loader" script and parse results.
    InvocationResult loadProfiles(const std::string& filePath) const;

private:
    std::string buildCommandLine(const CsScriptDescriptor& desc,
                                 const std::vector<std::string>& args) const;

    InvocationResult runProcess(const std::string& cmd) const;

    InvocationResult parseProfileOutput(const std::string& output) const;

    std::string m_dotnetExe;
    std::map<std::string, CsScriptDescriptor> m_registry;
};

} // namespace App