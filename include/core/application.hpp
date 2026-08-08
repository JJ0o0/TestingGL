#pragma once

#include <platform/window.hpp>
#include <core/game.hpp>
#include <optional>
#include <memory>

class Application {
    public:
        int Run();
    private:
        std::unique_ptr<Window> m_window;
        std::optional<Game> m_game;

        void stop();
};
