#pragma once

#include <uuid.h>

#include <functional>
#include <algorithm>
#include <random>
#include <array>

inline uuids::uuid GenerateUUID() {
    static std::random_device rd;
    static std::array<int, std::mt19937::state_size> seedData = [] {
        std::array<int, std::mt19937::state_size> data{};
        std::generate(data.begin(), data.end(), std::ref(rd));

        return data;
    }();

    static std::seed_seq seed(seedData.begin(), seedData.end());
    static std::mt19937 engine(seed);
    static uuids::uuid_random_generator generator{engine};

    return generator();
}

inline std::string UUIDToString(const uuids::uuid& uuid) {
    return uuids::to_string(uuid);
}
