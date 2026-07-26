cmake_minimum_required(VERSION 3.18)
project(UserProfileProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Output directories ────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Dependencies ──────────────────────────────────────────────────────────
find_package(nlohmann_json REQUIRED)

# ── Sub-projects ──────────────────────────────────────────────────────────
add_subdirectory(model)
add_subdirectory(app)

# ── Install targets (each target is installed from its own subdirectory) ──
# Individual CMakeLists handle install if needed.