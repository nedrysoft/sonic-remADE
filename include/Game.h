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

#ifndef NEDRYSOFT_GAME_H
#define NEDRYSOFT_GAME_H

#include "Errors.h"
#include "Structs.h"

#include <SDL2/SDL.h>

namespace Nedrysoft {
    class Game {
        public:
            static auto getInstance() -> Game *;

            static auto eventHandler() -> void;

            auto initialise() -> bool;

            auto initialiseRenderer(int width, int height, float scaleFactor) -> ErrorCodes::ErrorCode;

        private:
            Game();

        private:

            Nedrysoft::ISize m_size;

            SDL_Rect m_windowRenderRect;
            SDL_Rect m_gameRenderRect;

            float m_scaleFactor;

            SDL_Texture *m_renderTexture;
    };

}

auto handleEvents() -> bool;

#endif //NEDRYSOFT_GAME_H
