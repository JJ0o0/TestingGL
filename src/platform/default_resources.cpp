#include <platform/default_resources.hpp>

std::shared_ptr<Texture> DefaultResources::s_whiteTexture;
std::shared_ptr<Texture> DefaultResources::s_blackTexture;
std::shared_ptr<Texture> DefaultResources::s_flatTexture;

void DefaultResources::Initialize() {
    ImageData white {
        .Pixels = {255, 255, 255, 255},
        .Width = 1, .Height = 1
    };

    ImageData black {
        .Pixels = {0, 0, 0, 255},
        .Width = 1, .Height = 1
    };

    ImageData flat {
        .Pixels = {128, 128, 255, 255},
        .Width = 1, .Height = 1
    };

    s_whiteTexture = std::make_shared<Texture>(white, TextureFormat::RGBA8);
    s_blackTexture = std::make_shared<Texture>(black, TextureFormat::RGBA8);
    s_flatTexture = std::make_shared<Texture>(flat, TextureFormat::RGBA8);
}

void DefaultResources::Shutdown() {
    s_flatTexture.reset();
    s_blackTexture.reset();
    s_whiteTexture.reset();
}
