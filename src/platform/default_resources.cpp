#include <platform/default_resources.hpp>

std::shared_ptr<Texture> DefaultResources::s_whiteTexture;

void DefaultResources::Initialize() {
    ImageData white {
        .Pixels = {255, 255, 255, 255},
        .Width = 1, .Height = 1
    };

    s_whiteTexture = std::make_shared<Texture>(white, TextureFormat::RGBA8);
}

void DefaultResources::Shutdown() {
    s_whiteTexture.reset();
}
