#pragma once
#include "UserStore.h"
#include "CsBridgeInvoker.h"
#include "Config.h"
#include <string>
#include <filesystem>

namespace App::Menus {

// ── Entry point ───────────────────────────────────────────────────────────
void runMainMenu(UserStore& store,
                 CsBridgeInvoker& bridge,
                 const UserProfileModel::AppConfig& config,
                 const std::filesystem::path& exeDir);

// ── Sub-menus ─────────────────────────────────────────────────────────────
void runUserManagementMenu(UserStore& store, CsBridgeInvoker& bridge, const std::filesystem::path& exeDir);
void runViewUsersMenu(const UserStore& store);
void runCsBridgeMenu(UserStore& store,
                     CsBridgeInvoker& bridge,
                     const std::filesystem::path& exeDir);

// ── Leaf screens ─────────────────────────────────────────────────────────
void screenAddUser(UserStore& store);
void screenListUsers(const UserStore& store);
void screenViewUser(const UserStore& store);
void screenEditUser(UserStore& store, CsBridgeInvoker& bridge, const std::filesystem::path& exeDir);
void screenLoadFromFile(UserStore& store,
                        CsBridgeInvoker& bridge,
                        const std::filesystem::path& exeDir);
void screenListRegisteredScripts(const CsBridgeInvoker& bridge);

} // namespace App::Menus