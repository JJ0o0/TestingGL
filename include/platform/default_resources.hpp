#pragma once

#include <graphics/texture.hpp>
#include <memory>

class DefaultResources {
    public:
        static void Initialize();
        static void Shutdown();

        static inline std::shared_ptr<Texture> WhiteTexture() { return s_whiteTexture; }
    private:
        static std::shared_ptr<Texture> s_whiteTexture;
};
