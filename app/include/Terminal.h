#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

namespace App::Terminal {

// ── ANSI colour/style codes ───────────────────────────────────────────────
namespace Fg {
    constexpr const char* Reset       = "\033[0m";
    constexpr const char* Bold        = "\033[1m";
    constexpr const char* Dim         = "\033[2m";
    constexpr const char* Black       = "\033[30m";
    constexpr const char* Red         = "\033[31m";
    constexpr const char* Green       = "\033[32m";
    constexpr const char* Yellow      = "\033[33m";
    constexpr const char* Blue        = "\033[34m";
    constexpr const char* Magenta     = "\033[35m";
    constexpr const char* Cyan        = "\033[36m";
    constexpr const char* White       = "\033[37m";
    constexpr const char* BrightCyan  = "\033[96m";
    constexpr const char* BrightGreen = "\033[92m";
    constexpr const char* BrightBlue  = "\033[94m";
    constexpr const char* BrightWhite = "\033[97m";
}

namespace Bg {
    constexpr const char* Blue    = "\033[44m";
    constexpr const char* Cyan    = "\033[46m";
    constexpr const char* Black   = "\033[40m";
    constexpr const char* Magenta = "\033[45m";
}

inline void clearScreen() {
    std::cout << "\033[2J\033[H";
}

inline void moveTo(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

// Print a horizontal rule using a UTF-8 box-drawing string repeated 'width' times
inline void printHRule(const std::string& ch = "-", int width = 72,
                        const char* colour = Fg::Cyan) {
    std::cout << colour;
    for (int i = 0; i < width; ++i) std::cout << ch;
    std::cout << Fg::Reset << "\n";
}

inline void printBanner(const std::string& title,
                         const char* borderColour = Fg::BrightCyan,
                         const char* textColour   = Fg::Bold) {
    const int W = 72;
    std::cout << "\n";
    printHRule("=", W, borderColour);
    int pad = (W - static_cast<int>(title.size())) / 2;
    std::cout << borderColour << "|" << Fg::Reset
              << textColour
              << std::string(pad > 0 ? pad : 1, ' ')
              << title
              << std::string(pad > 0 ? pad : 1, ' ')
              << Fg::Reset
              << borderColour << "|" << Fg::Reset << "\n";
    printHRule("=", W, borderColour);
    std::cout << "\n";
}

inline void printSuccess(const std::string& msg) {
    std::cout << Fg::BrightGreen << "  ✔  " << msg << Fg::Reset << "\n";
}

inline void printError(const std::string& msg) {
    std::cout << Fg::Red << "  ✖  " << msg << Fg::Reset << "\n";
}

inline void printInfo(const std::string& msg) {
    std::cout << Fg::BrightCyan << "  ℹ  " << msg << Fg::Reset << "\n";
}

// Simple padded cell for table rendering
inline std::string cell(const std::string& s, int width) {
    if (static_cast<int>(s.size()) >= width)
        return s.substr(0, width - 1) + " ";
    return s + std::string(width - s.size(), ' ');
}

// Prompt and read a line
inline std::string prompt(const std::string& label,
                           const char* colour = Fg::Yellow) {
    std::cout << colour << "  » " << label << ": " << Fg::Reset;
    std::string input;
    std::getline(std::cin, input);
    // Trim trailing \r for safety
    if (!input.empty() && input.back() == '\r') input.pop_back();
    return input;
}

// Prompt with a default value
inline std::string promptOpt(const std::string& label,
                              const std::string& defaultVal = "",
                              const char* colour = Fg::Yellow) {
    std::string display = label;
    if (!defaultVal.empty()) display += " [" + defaultVal + "]";
    std::string v = prompt(display, colour);
    return v.empty() ? defaultVal : v;
}

// Print a labelled field
inline void field(const std::string& label, const std::string& value,
                  int labelWidth = 18,
                  const char* lc = Fg::Cyan,
                  const char* vc = Fg::BrightWhite) {
    std::string padded = label + ":";
    std::cout << "  " << lc << cell(padded, labelWidth) << Fg::Reset
              << vc << value << Fg::Reset << "\n";
}

} // namespace App::Terminal