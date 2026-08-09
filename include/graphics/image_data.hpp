#pragma once

#include <cstdint>
#include <vector>

struct ImageData {
    std::vector<uint8_t> Pixels;

    uint32_t Width = 0;
    uint32_t Height = 0;
};

struct HDRImageData {
    std::vector<float> Pixels;

    uint32_t Width = 0;
    uint32_t Height = 0;
};
