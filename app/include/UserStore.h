#pragma once
#include "UserProfile.h"
#include <vector>
#include <optional>
#include <string>
#include <algorithm>

namespace App {

class UserStore {
public:
    using ProfileList = std::vector<UserProfileModel::UserProfile>;

    void add(UserProfileModel::UserProfile profile);

    // Dynamic reflection patching updates the instance directly inside the store
    void update(UserProfileModel::UserProfile profile);

    [[nodiscard]] const ProfileList& all() const { return m_profiles; }

    [[nodiscard]] std::optional<UserProfileModel::UserProfile>
    findById(const std::string& id) const;

    [[nodiscard]] std::optional<UserProfileModel::UserProfile>
    findByUsername(const std::string& username) const;

    [[nodiscard]] bool exists(const std::string& id) const;

    [[nodiscard]] std::size_t size() const { return m_profiles.size(); }

    void clear() { m_profiles.clear(); }

private:
    ProfileList m_profiles;
};

} // namespace App