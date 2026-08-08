#pragma once

#include <stdexcept>
#include <format>

static inline void CheckError(bool trueCondition, const char* title, const char* description) {
    if (!trueCondition) {
        throw std::runtime_error(
            std::format("({}): {}", title, description)
        );
    }
}

template<typename... Args>
inline void CheckError(bool trueCondition, const char* title, std::format_string<Args...> fmt, Args&&... args) {
    if (!trueCondition) {
        throw std::runtime_error(
            std::format("({}): {}", title, std::format(fmt, std::forward<Args>(args)...))
        );
    }
}
