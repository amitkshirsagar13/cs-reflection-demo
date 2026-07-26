cmake_minimum_required(VERSION 3.18)
project(app VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── nlohmann/json (header-only) ──────────────────────────────────────────
find_package(nlohmann_json REQUIRED)

# ── Sources ───────────────────────────────────────────────────────────────
add_executable(user_profile_app
    src/main.cpp
    src/UserStore.cpp
    src/CsBridgeInvoker.cpp
    src/Menus.cpp
)

target_include_directories(user_profile_app
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link model library
target_link_libraries(user_profile_app
    PRIVATE
        model  # FIXED: Changed from 'userprofile_model' to the actual internal target name
        nlohmann_json::nlohmann_json
)

target_compile_options(user_profile_app PRIVATE
    -Wall -Wextra -Wpedantic
)