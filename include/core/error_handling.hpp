#pragma once

#include <stdexcept>
#include <format>

static inline void CheckError(bool trueCondition, const char* title, const char* description) {
    if (!trueCondition) {
        std::string formatted = std::format("({}): {}", title, description);
        const char* formattedSource = formatted.c_str();
        throw std::runtime_error(formattedSource);
    }
}
