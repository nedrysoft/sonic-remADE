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

#ifndef NEDRYSOFT_TEXTURE_H
#define NEDRYSOFT_TEXTURE_H

#include <Magick++.h>
#include <SDL2/SDL_render.h>

namespace Nedrysoft {
    class Texture {
        public:
            explicit Texture(SDL_Texture *texture, uint32_t *pixelData, int width, int height);

            ~Texture();

            auto texture() -> SDL_Texture *;

            auto data() -> uint32_t *;

            [[nodiscard]] auto width() const -> int;

            [[nodiscard]] auto height() const -> int;

        private:
            SDL_Texture *m_texture;

            uint32_t *m_data;

            int m_width;
            int m_height;
    };
}

#endif //NEDRYSOFT_TEXTURE_H
