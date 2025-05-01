/**
 * Copyright (C) 2025 Adrian Carpenter
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * This file is part of Sonic remADE, a C++ implementation of a
 * Sonic The Hedgehog 1 game engine.
 *
 * https://github.com/nedrysoft/sonic-remade
 */

#ifndef NEDRYSOFT_GAMERENDERER_H
#define NEDRYSOFT_GAMERENDERER_H

#include "Structs.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_render.h>
#include <chrono>
#include <list>

auto constexpr MicrosecondsPerFrame = (1.0 / 60.0) * 1000000.0;

namespace Nedrysoft {
    class GameRenderer {
        public:
            static auto getInstance() -> GameRenderer *;

            auto initialise(SDL_Window *window, SDL_Renderer *renderer, int viewportWidth, int viewportHeight, float viewportScale) -> void;

            auto setDefaultFont(TTF_Font *font) -> void;
            [[nodiscard]] auto defaultFont() const -> TTF_Font *;

            [[nodiscard]] auto viewportWidth() const -> int;
            [[nodiscard]] auto viewportHeight() const -> int;
            [[nodiscard]] auto viewportScale() const -> float;

            [[nodiscard]] auto viewportSize() const -> Nedrysoft::Size;

            auto renderer() -> SDL_Renderer *;
            auto window() -> SDL_Window *;

            auto beginRendering() -> void;
            auto endRendering() -> void;

            auto averageFramesPerSecond() const -> float;
            auto averageFrameDuration() const -> float;

            auto startFrameTimer() -> void;
            auto endFrameTimer() -> void;
            auto frameTime() -> double;
            auto waitForNextFrame() -> void;

        private:
            GameRenderer() = default;
            ~GameRenderer() = default;

        private:
            SDL_Renderer *m_renderer;
            SDL_Window *m_window;

            int m_viewportWidth;
            int m_viewportHeight;

            float m_viewportScale;

            TTF_Font *m_defaultFont;

            std::list<float> m_frameDurations;

            long m_startTick;
            long m_endTick;

            float m_averageFrameDuration;
            float m_averageFramesPerSecond;

            std::chrono::time_point<std::chrono::steady_clock> m_frameStartMicrosecond;
            std::chrono::time_point<std::chrono::steady_clock> m_frameEndMicrosecond;
            /*
            uint32_t *m_backgroundTilemapBuffer;
            uint32_t *m_foregroundTilemapBuffer;
            uint32_t *m_spriteBuffer;
            uint32_t *m_prioritySpriteBuffer;
            */
    };
}

#endif //NEDRYSOFT_GAMERENDERER_H
