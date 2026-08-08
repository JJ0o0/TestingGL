#pragma once

#include <graphics/texture.hpp>
#include <memory>

class DefaultResources {
    public:
        static void Initialize();
        static void Shutdown();

        static inline std::shared_ptr<Texture> WhiteTexture() { return s_whiteTexture; }
        static inline std::shared_ptr<Texture> BlackTexture() { return s_blackTexture; }
        static inline std::shared_ptr<Texture> FlatNormalTexture() { return s_flatTexture; }
    private:
        static std::shared_ptr<Texture> s_whiteTexture;
        static std::shared_ptr<Texture> s_blackTexture;
        static std::shared_ptr<Texture> s_flatTexture;
};
