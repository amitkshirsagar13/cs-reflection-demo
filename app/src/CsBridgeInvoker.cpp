#include "CsBridgeInvoker.h"
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <array>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace App {

CsBridgeInvoker::CsBridgeInvoker(std::string dotnetExe)
    : m_dotnetExe(std::move(dotnetExe)) {}

// ── Registry ─────────────────────────────────────────────────────────────

void CsBridgeInvoker::registerScript(CsScriptDescriptor descriptor) {
    LOG_INFO("CsBridgeInvoker: registering script '" + descriptor.name + "'");
    LOG_DEBUG("  dll=" + descriptor.dllPath +
              " type=" + descriptor.typeName);
    m_registry[descriptor.name] = std::move(descriptor);
}

bool CsBridgeInvoker::isRegistered(const std::string& name) const {
    return m_registry.count(name) > 0;
}

const CsScriptDescriptor&
CsBridgeInvoker::getDescriptor(const std::string& name) const {
    auto it = m_registry.find(name);
    if (it == m_registry.end())
        throw std::runtime_error("CsBridgeInvoker: unknown script '" + name + "'");
    return it->second;
}

std::vector<std::string> CsBridgeInvoker::registeredNames() const {
    std::vector<std::string> names;
    names.reserve(m_registry.size());
    for (auto& [k, _] : m_registry) names.push_back(k);
    return names;
}

// ── Invocation ───────────────────────────────────────────────────────────

InvocationResult CsBridgeInvoker::invoke(const std::string& scriptName,
                                          const std::vector<std::string>& args) const {
    if (!isRegistered(scriptName)) {
        return {false, "Script '" + scriptName + "' is not registered.", {}, -1};
    }
    const auto& desc = getDescriptor(scriptName);
    std::string cmd  = buildCommandLine(desc, args);
    LOG_INFO("CsBridgeInvoker: invoking '" + scriptName + "'");
    LOG_DEBUG("  cmd=" + cmd);
    return runProcess(cmd);
}

InvocationResult CsBridgeInvoker::loadProfiles(const std::string& filePath) const {
    auto result = invoke("profile_loader", {filePath});
    if (!result.success) return result;
    return parseProfileOutput(
        // re-run to capture stdout (runProcess already captured it)
        result.errorMessage.empty() ? result.errorMessage : ""
    );
    // The parseProfileOutput is called inside runProcess when we detect JSON
    return result;
}

std::string CsBridgeInvoker::buildCommandLine(const CsScriptDescriptor& desc,
                                               const std::vector<std::string>& args) const {
    std::ostringstream oss;
    oss << m_dotnetExe << " exec \"" << desc.dllPath << "\"";
    for (auto& a : args) {
        // Enforce strong quoting so complex inner JSON payloads aren't split by shell tokenizers
        oss << " \"" << a << "\""; 
    }
    return oss.str();
}

InvocationResult CsBridgeInvoker::runProcess(const std::string& cmd) const {
    InvocationResult result;
    std::string      output;

    std::array<char, 512> buf{};
    // popen captures stdout; stderr goes to terminal (good for debugging)
    FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
    if (!pipe) {
        result.success      = false;
        result.errorMessage = "popen() failed for command: " + cmd;
        return result;
    }

    while (fgets(buf.data(), buf.size(), pipe) != nullptr) {
        output += buf.data();
    }
    result.exitCode = pclose(pipe);
    result.success  = (result.exitCode == 0);

    LOG_DEBUG("CsBridgeInvoker: exit=" + std::to_string(result.exitCode));

    if (!result.success) {
        result.errorMessage = "Process exited with code " +
                              std::to_string(result.exitCode) +
                              "\nOutput:\n" + output;
        return result;
    }

    // Try to parse JSON array of profiles from output
    // The C# script emits a JSON block delimited by markers
    const std::string START = "---PROFILES_JSON_START---";
    const std::string END   = "---PROFILES_JSON_END---";
    auto sPos = output.find(START);
    auto ePos = output.find(END);

    if (sPos != std::string::npos && ePos != std::string::npos) {
        std::string jsonStr = output.substr(sPos + START.size(),
                                            ePos - sPos - START.size());
        try {
            auto arr = json::parse(jsonStr);
            for (auto& obj : arr) {
                UserProfileModel::UserProfileBuilder b;
                b.withId(obj.value("id", ""))
                 .withFirstName(obj.value("firstName", ""))
                 .withLastName(obj.value("lastName", ""))
                 .withUsername(obj.value("username", ""))
                 .withPassword(obj.value("password", ""))
                 .withAge(obj.value("age", 0));

                for (auto& e : obj.value("emails", json::array()))
                    b.addEmail(e.get<std::string>());
                for (auto& m : obj.value("mobiles", json::array()))
                    b.addMobile(m.get<std::string>());

                if (obj.contains("address")) {
                    auto& a = obj["address"];
                    UserProfileModel::Address addr;
                    addr.firstLine = a.value("firstLine", "");
                    addr.aptUnit   = a.value("aptUnit",   "");
                    addr.city      = a.value("city",      "");
                    addr.state     = a.value("state",     "");
                    addr.zip       = a.value("zip",       "");
                    b.withAddress(addr);
                }

                try {
                    result.profiles.push_back(b.build());
                } catch (std::exception& ex) {
                    LOG_WARN(std::string("Skipping invalid profile: ") + ex.what());
                }
            }
            LOG_INFO("CsBridgeInvoker: parsed " +
                     std::to_string(result.profiles.size()) + " profiles");
        } catch (std::exception& ex) {
            result.success      = false;
            result.errorMessage = std::string("JSON parse error: ") + ex.what();
        }
    } else {
        LOG_DEBUG("CsBridgeInvoker: no profile JSON markers found in output");
    }

    return result;
}

InvocationResult
CsBridgeInvoker::parseProfileOutput(const std::string& /*output*/) const {
    // This overload is unused; the full parse happens inside runProcess.
    return {};
}

} // namespace App