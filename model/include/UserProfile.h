#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace UserProfileModel {

// ─────────────────────────────────────────────
//  Address
// ─────────────────────────────────────────────
struct Address {
    std::string firstLine;
    std::string aptUnit;        // Apt / Unit (optional)
    std::string city;
    std::string state;
    std::string zip;

    Address() = default;
    Address(std::string firstLine,
            std::string aptUnit,
            std::string city,
            std::string state,
            std::string zip)
        : firstLine(std::move(firstLine))
        , aptUnit(std::move(aptUnit))
        , city(std::move(city))
        , state(std::move(state))
        , zip(std::move(zip)) {}
};

// ─────────────────────────────────────────────
//  UserProfile
// ─────────────────────────────────────────────
struct UserProfile {
    std::string              id;
    std::string              firstName;
    std::string              lastName;
    std::string              username;
    std::string              password;     // stored hashed in real systems
    std::vector<std::string> emails;
    std::vector<std::string> mobiles;
    Address                  address;
    int                      age{0};

    UserProfile() = default;

    // Computed helpers
    [[nodiscard]] std::string fullName() const {
        return firstName + " " + lastName;
    }
};

// ─────────────────────────────────────────────
//  UserProfile Builder
// ─────────────────────────────────────────────
class UserProfileBuilder {
public:
    UserProfileBuilder& withId(std::string id);
    UserProfileBuilder& withFirstName(std::string firstName);
    UserProfileBuilder& withLastName(std::string lastName);
    UserProfileBuilder& withUsername(std::string username);
    UserProfileBuilder& withPassword(std::string password);
    UserProfileBuilder& addEmail(std::string email);
    UserProfileBuilder& addMobile(std::string mobile);
    UserProfileBuilder& withAddress(Address address);
    UserProfileBuilder& withAge(int age);

    [[nodiscard]] UserProfile build() const;
    void                      reset();

private:
    UserProfile m_profile;
};

} // namespace UserProfileModel