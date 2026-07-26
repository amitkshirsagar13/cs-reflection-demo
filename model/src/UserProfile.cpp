#include "UserProfile.h"
#include "Logger.h"
#include <stdexcept>

namespace UserProfileModel {

// ─────────────────────────────────────────────
//  Builder
// ─────────────────────────────────────────────
UserProfileBuilder& UserProfileBuilder::withId(std::string id) {
    m_profile.id = std::move(id);
    return *this;
}

UserProfileBuilder& UserProfileBuilder::withFirstName(std::string firstName) {
    m_profile.firstName = std::move(firstName);
    return *this;
}

UserProfileBuilder& UserProfileBuilder::withLastName(std::string lastName) {
    m_profile.lastName = std::move(lastName);
    return *this;
}

UserProfileBuilder& UserProfileBuilder::withUsername(std::string username) {
    m_profile.username = std::move(username);
    return *this;
}

UserProfileBuilder& UserProfileBuilder::withPassword(std::string password) {
    m_profile.password = std::move(password);
    return *this;
}

UserProfileBuilder& UserProfileBuilder::addEmail(std::string email) {
    m_profile.emails.push_back(std::move(email));
    return *this;
}

UserProfileBuilder& UserProfileBuilder::addMobile(std::string mobile) {
    m_profile.mobiles.push_back(std::move(mobile));
    return *this;
}

UserProfileBuilder& UserProfileBuilder::withAddress(Address address) {
    m_profile.address = std::move(address);
    return *this;
}

UserProfileBuilder& UserProfileBuilder::withAge(int age) {
    if (age < 0 || age > 150) {
        LOG_WARN("Age out of plausible range: " + std::to_string(age));
    }
    m_profile.age = age;
    return *this;
}

UserProfile UserProfileBuilder::build() const {
    if (m_profile.id.empty()) {
        throw std::runtime_error("UserProfile must have a non-empty id");
    }
    if (m_profile.username.empty()) {
        throw std::runtime_error("UserProfile must have a non-empty username");
    }
    LOG_DEBUG("Built UserProfile id=" + m_profile.id +
              " username=" + m_profile.username);
    return m_profile;
}

void UserProfileBuilder::reset() {
    m_profile = UserProfile{};
}

} // namespace UserProfileModel