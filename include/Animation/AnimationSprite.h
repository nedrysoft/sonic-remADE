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

#ifndef NEDRYSOFT_ANIMATIONSPRITE_H
#define NEDRYSOFT_ANIMATIONSPRITE_H

#include "Structs.h"

#include <SDL2/SDL.h>
#include <string>

namespace Nedrysoft {
    class AnimationSprite {
        public:
            AnimationSprite(std::string name, Vector origin, int width, int height, SDL_Texture *texture);

            auto origin() -> Vector;

            [[nodiscard]] auto width() const -> int;
            [[nodiscard]] auto height() const -> int;

            auto texture() -> SDL_Texture *;
            auto name() -> std::string;

        private:
            Vector m_origin;

            int m_width;
            int m_height;

            SDL_Texture *m_texture;

            std::string m_name;
    };
}

#endif //NEDRYSOFT_ANIMATIONSPRITE_H
