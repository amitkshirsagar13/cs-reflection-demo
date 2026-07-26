cmake_minimum_required(VERSION 3.18)
project(model VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Sources ───────────────────────────────────
add_library(model SHARED
    src/UserProfile.cpp
    src/Logger.cpp
    src/Config.cpp
)

target_include_directories(model
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Threads for the mutex in Logger
find_package(Threads REQUIRED)
target_link_libraries(model PRIVATE Threads::Threads)

# Compiler warnings
target_compile_options(model PRIVATE
    -Wall -Wextra -Wpedantic
)

set_target_properties(model PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "userprofile_model"
)