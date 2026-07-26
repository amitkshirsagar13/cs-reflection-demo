#include "Menus.h"
#include "Terminal.h"
#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using namespace App::Terminal;
using namespace UserProfileModel;

namespace App::Menus {

// ────────────────────────────────────────────────────────────────────────
//  Helpers
// ────────────────────────────────────────────────────────────────────────

static void waitEnter() {
    std::cout << "\n" << Fg::Dim
              << "  Press ENTER to continue..." << Fg::Reset;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static int readChoice(int lo, int hi) {
    while (true) {
        std::cout << Fg::Yellow << "\n  Choice [" << lo << "-" << hi << "]: "
                  << Fg::Reset;
        std::string line;
        std::getline(std::cin, line);
        try {
            int v = std::stoi(line);
            if (v >= lo && v <= hi) return v;
        } catch (...) {}
        printError("Invalid input, please enter " +
                   std::to_string(lo) + "–" + std::to_string(hi));
    }
}

static void printMenuOption(int num, const std::string& label,
                             const char* numColour = Fg::BrightCyan) {
    std::cout << "  " << numColour << "[" << num << "] "
              << Fg::BrightWhite << label << Fg::Reset << "\n";
}

static void printSectionTitle(const std::string& title) {
    std::cout << "\n" << Fg::Magenta << Fg::Bold
              << "  ── " << title << " ──" << Fg::Reset << "\n\n";
}

// ────────────────────────────────────────────────────────────────────────
//  UserProfile display helpers
// ────────────────────────────────────────────────────────────────────────

static void printUserTable(const UserStore::ProfileList& profiles) {
    if (profiles.empty()) {
        printInfo("No users in store.");
        return;
    }

    // Column widths
    constexpr int W_ID   = 12;
    constexpr int W_USER = 18;
    constexpr int W_NAME = 24;
    constexpr int W_AGE  =  6;
    constexpr int W_EMAIL= 28;

    auto hdr = [&](const char* label, int w) {
        std::cout << Fg::Bold << Fg::BrightCyan
                  << cell(label, w) << Fg::Reset;
    };

    printHRule("-", W_ID + W_USER + W_NAME + W_AGE + W_EMAIL + 4);
    std::cout << "  ";
    hdr("ID",       W_ID);
    hdr("Username", W_USER);
    hdr("Full Name",W_NAME);
    hdr("Age",      W_AGE);
    hdr("Primary Email", W_EMAIL);
    std::cout << "\n";
    printHRule("-", W_ID + W_USER + W_NAME + W_AGE + W_EMAIL + 4);

    for (auto& p : profiles) {
        std::string primaryEmail = p.emails.empty() ? "-" : p.emails[0];
        std::cout << "  "
                  << Fg::Cyan  << cell(p.id,          W_ID)
                  << Fg::White << cell(p.username,    W_USER)
                  << Fg::BrightWhite << cell(p.fullName(),W_NAME)
                  << Fg::Yellow<< cell(std::to_string(p.age), W_AGE)
                  << Fg::Dim   << cell(primaryEmail,  W_EMAIL)
                  << Fg::Reset << "\n";
    }
    printHRule("-", W_ID + W_USER + W_NAME + W_AGE + W_EMAIL + 4);
    std::cout << "  " << Fg::Dim << profiles.size()
              << " user(s)" << Fg::Reset << "\n";
}

static void printUserProfile(const UserProfile& p) {
    printBanner("User Profile: " + p.fullName(), Fg::BrightBlue, Fg::Bold);

    printSectionTitle("Identity");
    field("ID",        p.id);
    field("Username",  p.username);
    field("Full Name", p.fullName());
    field("Age",       std::to_string(p.age));

    printSectionTitle("Contact");
    if (p.emails.empty()) {
        field("Email", "(none)");
    } else {
        for (std::size_t i = 0; i < p.emails.size(); ++i)
            field("Email " + std::to_string(i+1), p.emails[i]);
    }
    if (p.mobiles.empty()) {
        field("Mobile", "(none)");
    } else {
        for (std::size_t i = 0; i < p.mobiles.size(); ++i)
            field("Mobile " + std::to_string(i+1), p.mobiles[i]);
    }

    printSectionTitle("Address");
    field("First Line", p.address.firstLine.empty() ? "(none)" : p.address.firstLine);
    if (!p.address.aptUnit.empty()) field("Apt/Unit", p.address.aptUnit);
    field("City",  p.address.city.empty()  ? "(none)" : p.address.city);
    field("State", p.address.state.empty() ? "(none)" : p.address.state);
    field("ZIP",   p.address.zip.empty()   ? "(none)" : p.address.zip);

    printHRule("-");
}

// ────────────────────────────────────────────────────────────────────────
//  Leaf screens
// ────────────────────────────────────────────────────────────────────────

void screenAddUser(UserStore& store) {
    clearScreen();
    printBanner("Add New User", Fg::BrightGreen);

    UserProfileBuilder builder;

    // Required
    std::string id = "";
    while (id.empty()) id = prompt("User ID (unique)");
    builder.withId(id);

    std::string username = "";
    while (username.empty()) username = prompt("Username");
    builder.withUsername(username);

    builder.withFirstName(promptOpt("First Name"));
    builder.withLastName(promptOpt("Last Name"));
    builder.withPassword(promptOpt("Password"));

    std::string ageStr = promptOpt("Age", "0");
    try { builder.withAge(std::stoi(ageStr)); } catch (...) {}

    // Emails
    printSectionTitle("Emails  (empty to stop)");
    for (int i = 1; ; ++i) {
        std::string e = promptOpt("Email " + std::to_string(i));
        if (e.empty()) break;
        builder.addEmail(e);
    }

    // Mobiles
    printSectionTitle("Mobiles  (empty to stop)");
    for (int i = 1; ; ++i) {
        std::string m = promptOpt("Mobile " + std::to_string(i));
        if (m.empty()) break;
        builder.addMobile(m);
    }

    // Address
    printSectionTitle("Address  (empty to skip)");
    Address addr;
    addr.firstLine = promptOpt("First Line");
    addr.aptUnit   = promptOpt("Apt / Unit");
    addr.city      = promptOpt("City");
    addr.state     = promptOpt("State");
    addr.zip       = promptOpt("ZIP");
    builder.withAddress(addr);

    try {
        auto profile = builder.build();
        store.add(profile);
        printSuccess("User '" + profile.username + "' added successfully.");
        LOG_INFO("User added: id=" + profile.id + " username=" + profile.username);
    } catch (std::exception& ex) {
        printError(std::string("Failed to create user: ") + ex.what());
        LOG_ERROR(std::string("screenAddUser: ") + ex.what());
    }

    waitEnter();
}

void screenListUsers(const UserStore& store) {
    clearScreen();
    printBanner("All Users", Fg::BrightCyan);
    printUserTable(store.all());
    waitEnter();
}

void screenViewUser(const UserStore& store) {
    clearScreen();
    printBanner("View User Profile", Fg::BrightBlue);

    if (store.size() == 0) {
        printInfo("No users in store.");
        waitEnter();
        return;
    }

    // Show quick ID list first
    std::cout << Fg::Dim << "  Available IDs:\n";
    for (auto& p : store.all())
        std::cout << "    " << Fg::Cyan << p.id
                  << Fg::Dim << "  (" << p.username << ")\n";
    std::cout << Fg::Reset << "\n";

    std::string key = prompt("Enter ID or Username");
    LOG_DEBUG("screenViewUser: searching key='" + key + "'");

    auto found = store.findById(key);
    if (!found) found = store.findByUsername(key);

    if (!found) {
        printError("No user found for: " + key);
    } else {
        printUserProfile(*found);
    }
    waitEnter();
}

void screenLoadFromFile(UserStore& store,
                         CsBridgeInvoker& bridge,
                         const fs::path& exeDir) {
    clearScreen();
    printBanner("Load Profiles from File  (C# Reflection Bridge)", Fg::Magenta);

    printInfo("Supported formats: .json, .yml, .yaml");
    printInfo("File will be looked up relative to: " + exeDir.string());
    std::cout << "\n";

    std::string fileName = prompt("File name (e.g. users.json)");
    if (fileName.empty()) {
        printError("No file name entered.");
        waitEnter();
        return;
    }

    fs::path filePath = exeDir / fileName;
    LOG_INFO("screenLoadFromFile: looking for " + filePath.string());

    if (!fs::exists(filePath)) {
        printError("File not found: " + filePath.string());
        waitEnter();
        return;
    }

    printInfo("Invoking C# bridge for: " + filePath.string());
    std::cout << "\n";

    auto result = bridge.invoke("profile_loader", {filePath.string()});

    if (!result.success) {
        printError("Bridge invocation failed.");
        std::cout << Fg::Red << result.errorMessage << Fg::Reset << "\n";
        LOG_ERROR("Bridge failed: " + result.errorMessage);
        waitEnter();
        return;
    }

    int added = 0;
    for (auto& p : result.profiles) {
        if (!store.exists(p.id)) {
            store.add(p);
            ++added;
        } else {
            LOG_WARN("Duplicate id '" + p.id + "' from file – skipping");
        }
    }

    printSuccess(std::to_string(result.profiles.size()) + " profile(s) parsed, " +
                 std::to_string(added) + " added to store.");
    waitEnter();
}

void screenListRegisteredScripts(const CsBridgeInvoker& bridge) {
    clearScreen();
    printBanner("Registered C# Scripts", Fg::Magenta);

    auto names = bridge.registeredNames();
    if (names.empty()) {
        printInfo("No scripts registered.");
        waitEnter();
        return;
    }

    for (auto& n : names) {
        const auto& d = bridge.getDescriptor(n);
        std::cout << "\n";
        field("Name",        d.name);
        field("DLL Path",    d.dllPath);
        field("Type",        d.typeName);
        field("Description", d.description);
    }
    waitEnter();
}
void screenEditUser(UserStore& store, CsBridgeInvoker& bridge, const fs::path& exeDir) {
    clearScreen();
    printBanner("Edit Existing User (Patch via C# Reflection)", Fg::Red);

    std::string userId = prompt("Enter User ID to edit");
    if (userId.empty()) return;

    auto found = store.findById(userId);
    if (!found) {
        printError("User with ID '" + userId + "' not found in store.");
        waitEnter();
        return;
    }

    // Lookup patch file
    fs::path patchPath;
    if (fs::exists(exeDir / (userId + ".json"))) {
        patchPath = exeDir / (userId + ".json");
    } else if (fs::exists(exeDir / (userId + ".yml"))) {
        patchPath = exeDir / (userId + ".yml");
    } else if (fs::exists(exeDir / (userId + ".yaml"))) {
        patchPath = exeDir / (userId + ".yaml");
    } else {
        printError("No patch file found (<userid>.json or <userid>.yml) in folder: " + exeDir.string());
        waitEnter();
        return;
    }

    printInfo("Found patch file: " + patchPath.filename().string());
    printInfo("Writing current profile state to temporary file for process bridge...");

    // Serialize existing C++ state to standard JSON
    nlohmann::json currentJson;
    currentJson["id"] = found->id;
    currentJson["firstName"] = found->firstName;
    currentJson["lastName"] = found->lastName;
    currentJson["username"] = found->username;
    currentJson["password"] = found->password;
    currentJson["age"] = found->age;
    currentJson["emails"] = found->emails;
    currentJson["mobiles"] = found->mobiles;
    
    nlohmann::json addrJson;
    addrJson["firstLine"] = found->address.firstLine;
    addrJson["aptUnit"] = found->address.aptUnit;
    addrJson["city"] = found->address.city;
    addrJson["state"] = found->address.state;
    addrJson["zip"] = found->address.zip;
    currentJson["address"] = addrJson;

    // Write state to a temporary JSON file to avoid shell string escaping issues
    fs::path tempStatePath = exeDir / (userId + ".tmp");
    std::ofstream tmpFile(tempStatePath);
    if (!tmpFile.is_open()) {
        printError("Failed to create temporary state file.");
        waitEnter();
        return;
    }
    tmpFile << currentJson.dump();
    tmpFile.close();

    // Pass BOTH the patch file path and the temporary state path
    auto result = bridge.invoke("profile_loader", { patchPath.string(), tempStatePath.string() });

    // Clean up the temporary file immediately
    std::error_code ec;
    fs::remove(tempStatePath, ec);

    if (!result.success || result.profiles.empty()) {
        printError("Patching failed.");
        std::cout << Fg::Red << result.errorMessage << Fg::Reset << "\n";
        waitEnter();
        return;
    }

    // Overwrite the user inside the container using our update method
    store.update(result.profiles[0]); 

    printSuccess("User profile successfully patched via Reflection.");
    waitEnter();
}

// ────────────────────────────────────────────────────────────────────────
//  Sub-menus
// ────────────────────────────────────────────────────────────────────────

void runUserManagementMenu(UserStore& store, CsBridgeInvoker& bridge, const fs::path& exeDir) {
    while (true) {
        clearScreen();
        printBanner("User Management", Fg::BrightGreen);
        printMenuOption(1, "Add New User");
        printMenuOption(2, "List All Users");
        printMenuOption(3, "View Single User");
        printMenuOption(4, "Edit Existing User (Patch File)");
        printMenuOption(0, "Back to Main Menu", Fg::Yellow);

        int choice = readChoice(0, 4);
        switch (choice) {
            case 1: screenAddUser(store);                            break;
            case 2: screenListUsers(store);                          break;
            case 3: screenViewUser(store);                           break;
            case 4: screenEditUser(store, bridge, exeDir);           break;
            case 0: return;
        }
    }
}

void runViewUsersMenu(const UserStore& store) {
    while (true) {
        clearScreen();
        printBanner("View Users", Fg::BrightCyan);
        printMenuOption(1, "List All Users (Table)");
        printMenuOption(2, "View Single User (Profile)");
        printMenuOption(0, "Back", Fg::Yellow);

        int choice = readChoice(0, 2);
        switch (choice) {
            case 1: screenListUsers(store); break;
            case 2: screenViewUser(store);  break;
            case 0: return;
        }
    }
}

void runCsBridgeMenu(UserStore& store,
                      CsBridgeInvoker& bridge,
                      const fs::path& exeDir) {
    while (true) {
        clearScreen();
        printBanner("C# Reflection Bridge", Fg::Magenta);
        printInfo("Use this menu to invoke C# scripts via the reflection bridge.");
        std::cout << "\n";
        printMenuOption(1, "Load User Profiles from JSON/YAML File");
        printMenuOption(2, "List Registered C# Scripts");
        printMenuOption(0, "Back", Fg::Yellow);

        int choice = readChoice(0, 2);
        switch (choice) {
            case 1: screenLoadFromFile(store, bridge, exeDir); break;
            case 2: screenListRegisteredScripts(bridge);        break;
            case 0: return;
        }
    }
}

// ────────────────────────────────────────────────────────────────────────
//  Main Menu
// ────────────────────────────────────────────────────────────────────────

void runMainMenu(UserStore& store,
                  CsBridgeInvoker& bridge,
                  const AppConfig& config,
                  const fs::path& exeDir) {
    while (true) {
        clearScreen();
        printBanner(config.appName + "  v" + config.version,
                    Fg::BrightCyan, Fg::Bold);

        std::cout << Fg::Dim << "  Users in store: "
                  << Fg::BrightWhite << store.size()
                  << Fg::Reset << "\n\n";

        printSectionTitle("Main Menu");
        printMenuOption(1, "User Management");
        printMenuOption(2, "View Users");
        printMenuOption(3, "C# Reflection Bridge  (Load from File)");
        printMenuOption(0, "Exit", Fg::Red);

        int choice = readChoice(0, 3);
        LOG_DEBUG("Main menu choice: " + std::to_string(choice));

        switch (choice) {
            case 1: runUserManagementMenu(store, bridge, exeDir); break;
            case 2: runViewUsersMenu(store);               break;
            case 3: runCsBridgeMenu(store, bridge, exeDir); break;
            case 0:
                clearScreen();
                std::cout << Fg::BrightCyan
                          << "\n  Goodbye!\n\n" << Fg::Reset;
                LOG_INFO("Application exiting normally.");
                return;
        }
    }
}

} // namespace App::Menus