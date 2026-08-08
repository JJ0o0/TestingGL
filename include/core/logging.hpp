#pragma once

#include <iostream>
#include <utility>
#include <format>

template<typename... Args>
static inline void LogInfo(std::format_string<Args...> fmt, Args&&... args) {
    std::cout << "[INFO] " << std::format(fmt, std::forward<Args>(args)...) << "\n";
}

template<typename... Args>
static inline void LogWarning(std::format_string<Args...> fmt, Args&&... args) {
    std::cerr << "[WARNING] " << std::format(fmt, std::forward<Args>(args)...) << "\n";
}

template<typename... Args>
static inline void LogError(std::format_string<Args...> fmt, Args&&... args) {
    std::cerr << "[ERROR] " << std::format(fmt, std::forward<Args>(args)...) << "\n";
}
