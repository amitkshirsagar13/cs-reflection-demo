#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
#  build.sh — Build and run the User Profile Project
#
#  Usage:
#    ./build.sh [OPTIONS]
#
#  Options:
#    -h | --help          Show this help and exit
#    -b | --build         Build C++ modules only (default if no flag given)
#    -c | --build-cs      Build C# bridge only
#    -a | --build-all     Build both C++ and C# (recommended first run)
#    -r | --run           Run the application after building
#    -R | --run-only      Run without building (assumes already built)
#    -d | --debug         Enable CMake Debug build type
#    -j | --jobs N        Number of parallel build jobs (default: nproc)
#    -C | --clean         Clean build artefacts before building
#    --release            Build in Release mode
#    --no-colour          Disable coloured output
# ═══════════════════════════════════════════════════════════════════════════

set -euo pipefail

# ── Defaults ──────────────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BIN_DIR="${BUILD_DIR}/bin"
CS_DIR="${SCRIPT_DIR}/csbridge/CsRunner"
CS_OUT_DIR="${BIN_DIR}/csbridge"

BUILD_CPP=false
BUILD_CS=false
RUN_APP=false
CLEAN=false
BUILD_TYPE="Debug"
JOBS="$(nproc 2>/dev/null || echo 4)"
USE_COLOUR=true

# ── Colours ───────────────────────────────────────────────────────────────
C_RESET="\033[0m"
C_BOLD="\033[1m"
C_CYAN="\033[36m"
C_GREEN="\033[32m"
C_YELLOW="\033[33m"
C_RED="\033[31m"
C_MAGENTA="\033[35m"

print_header() {
    if $USE_COLOUR; then
        echo -e "\n${C_CYAN}${C_BOLD}══════════════════════════════════════════════════${C_RESET}"
        echo -e "${C_CYAN}${C_BOLD}  $1${C_RESET}"
        echo -e "${C_CYAN}${C_BOLD}══════════════════════════════════════════════════${C_RESET}\n"
    else
        echo "=== $1 ==="
    fi
}

print_ok()   { $USE_COLOUR && echo -e "${C_GREEN}  ✔  $1${C_RESET}" || echo "  [OK]  $1"; }
print_info() { $USE_COLOUR && echo -e "${C_CYAN}  ℹ  $1${C_RESET}"  || echo "  [INFO] $1"; }
print_warn() { $USE_COLOUR && echo -e "${C_YELLOW}  ⚠  $1${C_RESET}" || echo "  [WARN] $1"; }
print_err()  { $USE_COLOUR && echo -e "${C_RED}  ✖  $1${C_RESET}"   || echo "  [ERR]  $1"; }

usage() {
    echo ""
    echo "  Usage: ./build.sh [OPTIONS]"
    echo ""
    echo "  Options:"
    echo "    -h | --help          Show this help and exit"
    echo "    -b | --build         Build C++ modules only"
    echo "    -c | --build-cs      Build C# bridge only"
    echo "    -a | --build-all     Build C++ and C# (recommended first run)"
    echo "    -r | --run           Build then run"
    echo "    -R | --run-only      Run without building"
    echo "    -d | --debug         CMake Debug build (default)"
    echo "    -j | --jobs N        Parallel build jobs (default: $(nproc))"
    echo "    -C | --clean         Clean before building"
    echo "    --release            CMake Release build"
    echo "    --no-colour          Disable ANSI colour output"
    echo ""
}

# ── Arg parsing ───────────────────────────────────────────────────────────
if [[ $# -eq 0 ]]; then
    usage
    exit 0
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)       usage; exit 0 ;;
        -b|--build)      BUILD_CPP=true ;;
        -c|--build-cs)   BUILD_CS=true ;;
        -a|--build-all)  BUILD_CPP=true; BUILD_CS=true ;;
        -r|--run)        BUILD_CPP=true; BUILD_CS=true; RUN_APP=true ;;
        -R|--run-only)   RUN_APP=true ;;
        -d|--debug)      BUILD_TYPE="Debug" ;;
        --release)       BUILD_TYPE="Release" ;;
        -C|--clean)      CLEAN=true ;;
        --no-colour)     USE_COLOUR=false ;;
        -j|--jobs)
            shift
            JOBS="$1"
            ;;
        *) print_err "Unknown option: $1"; usage; exit 1 ;;
    esac
    shift
done

# ── Prerequisite checks ───────────────────────────────────────────────────
check_tool() {
    if ! command -v "$1" &>/dev/null; then
        print_err "Required tool not found: $1"
        print_info "Install with: $2"
        exit 1
    fi
}

if $BUILD_CPP; then
    check_tool cmake  "sudo apt install cmake"
    check_tool ninja  "sudo apt install ninja-build  (or the make fallback will be used)"
    # nlohmann/json check happens at cmake configure time
fi

if $BUILD_CS; then
    check_tool dotnet "https://dotnet.microsoft.com/download"
fi

# ── Clean ─────────────────────────────────────────────────────────────────
if $CLEAN; then
    print_header "Cleaning"
    rm -rf "${BUILD_DIR}"
    print_ok "Build directory removed."
fi

# ── Build C++ ─────────────────────────────────────────────────────────────
if $BUILD_CPP; then
    print_header "Building C++ Modules  [${BUILD_TYPE}]"

    mkdir -p "${BUILD_DIR}"

    # Prefer ninja, fall back to make
    GENERATOR="Ninja"
    if ! command -v ninja &>/dev/null; then
        GENERATOR="Unix Makefiles"
        print_warn "ninja not found, falling back to Unix Makefiles"
    fi

    cmake -S "${SCRIPT_DIR}" \
          -B "${BUILD_DIR}" \
          -G "${GENERATOR}" \
          -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

    print_ok "C++ build complete → ${BIN_DIR}/"
fi

# ── Build C# ──────────────────────────────────────────────────────────────
if $BUILD_CS; then
    print_header "Building C# Bridge  [${BUILD_TYPE}]"

    mkdir -p "${CS_OUT_DIR}"

    # Change --configuration Release to use the dynamic BUILD_TYPE variable
    dotnet build "${CS_DIR}/CsRunner.csproj" \
        --configuration "${BUILD_TYPE}" \
        --output "${CS_OUT_DIR}"

    print_ok "C# build complete → ${CS_OUT_DIR}/"
fi

# ── Copy runtime assets into bin/ ─────────────────────────────────────────
if $BUILD_CPP || $BUILD_CS; then
    print_header "Staging Runtime Assets"

    # conf.yml
    if [[ -f "${SCRIPT_DIR}/config/conf.yml" ]]; then
        cp "${SCRIPT_DIR}/config/conf.yml" "${BIN_DIR}/conf.yml"
        print_ok "Copied conf.yml → ${BIN_DIR}/"
    fi

    # Sample files
    mkdir -p "${BIN_DIR}/samples"
    cp "${SCRIPT_DIR}"/samples/*.json "${BIN_DIR}/samples/" 2>/dev/null || true
    cp "${SCRIPT_DIR}"/samples/*.yml  "${BIN_DIR}/samples/" 2>/dev/null || true
    print_ok "Copied samples → ${BIN_DIR}/samples/"
fi

# ── Run ───────────────────────────────────────────────────────────────────
if $RUN_APP; then
    EXE="${BIN_DIR}/user_profile_app"
    if [[ ! -f "${EXE}" ]]; then
        print_err "Executable not found: ${EXE}"
        print_info "Run with --build-all first."
        exit 1
    fi

    print_header "Launching User Profile App"
    # Pass log level to the C# side via env var
    LOG_LEVEL="$(grep -A5 '\[logging\]' "${BIN_DIR}/conf.yml" 2>/dev/null | \
                 grep 'level' | head -1 | sed 's/.*=\s*//' | tr -d '[:space:]' || echo INFO)"
    export CSRUNNER_LOG_LEVEL="${LOG_LEVEL}"
    # export DOTNET_ROLL_FORWARD="Major" # Add this line here

    cp samples/*.json "${BIN_DIR}/" 2>/dev/null || true
    cp samples/*.yml "${BIN_DIR}/" 2>/dev/null || true

    cd "${BIN_DIR}"
    exec "${EXE}"
fi

print_header "Done"