#pragma once

#include <cstdint>

class Cubemap {
    public:
        Cubemap(uint32_t size, int internalFormat, int format, int type, bool mipmaps);
        ~Cubemap();

        Cubemap(const Cubemap&) = delete;
        Cubemap& operator=(const Cubemap&) = delete;

        Cubemap(Cubemap&& other) noexcept;
        Cubemap& operator=(Cubemap&& other) noexcept;

        void Bind(int slot = 0) const;

        uint32_t GetID() const { return m_id; }
        uint32_t GetSize() const { return m_size; }
    private:
        uint32_t m_id = 0;
        uint32_t m_size = 0;
};
