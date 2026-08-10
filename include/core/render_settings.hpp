#pragma once

enum class ToneMapping {
    None = 0,
    Reinhard = 1,
    ACES = 2
};

struct RenderSettings {
    ToneMapping Tonemapper = ToneMapping::ACES;
    float Exposure = 1.0f;
};
