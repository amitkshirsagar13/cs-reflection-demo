#include "UserStore.h"
#include "Logger.h"
#include <stdexcept>
#include <algorithm> // for std::find_if

namespace App {

void UserStore::add(UserProfileModel::UserProfile profile) {
    if (exists(profile.id)) { //[cite: 13]
        LOG_WARN("UserStore: duplicate id '" + profile.id + "' – skipping"); //[cite: 13]
        return; //[cite: 13]
    }
    LOG_DEBUG("UserStore: adding id=" + profile.id +
              " username=" + profile.username); //[cite: 13]
    m_profiles.push_back(std::move(profile)); //[cite: 13]
}

void UserStore::update(UserProfileModel::UserProfile profile) {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
        [&profile](const auto& p){ return p.id == profile.id; });

    if (it != m_profiles.end()) {
        LOG_DEBUG("UserStore: updating existing id=" + profile.id + " username=" + profile.username);
        *it = std::move(profile);
    } else {
        LOG_DEBUG("UserStore: target id=" + profile.id + " not found for update. Falling back to add.");
        m_profiles.push_back(std::move(profile));
    }
}

std::optional<UserProfileModel::UserProfile>
UserStore::findById(const std::string& id) const {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
        [&id](const auto& p){ return p.id == id; }); //[cite: 13]
    if (it == m_profiles.end()) return std::nullopt; //[cite: 13]
    return *it; //[cite: 13]
}

std::optional<UserProfileModel::UserProfile>
UserStore::findByUsername(const std::string& username) const {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
        [&username](const auto& p){ return p.username == username; }); //[cite: 13]
    if (it == m_profiles.end()) return std::nullopt; //[cite: 13]
    return *it; //[cite: 13]
}

bool UserStore::exists(const std::string& id) const {
    return std::any_of(m_profiles.begin(), m_profiles.end(),
        [&id](const auto& p){ return p.id == id; }); //[cite: 13]
}

} // namespace App