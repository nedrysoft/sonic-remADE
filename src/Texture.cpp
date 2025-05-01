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

#include "Texture.h"

Nedrysoft::Texture::Texture(SDL_Texture *texture, uint32_t *pixelData, int width, int height) {
    m_texture = texture;
    m_data = pixelData;
    m_width = width;
    m_height = height;
}

Nedrysoft::Texture::~Texture() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
    }
}

auto Nedrysoft::Texture::texture() -> SDL_Texture * {
    return m_texture;
}

auto Nedrysoft::Texture::data() -> uint32_t * {
    return m_data;
}

auto Nedrysoft::Texture::width() const -> int {
    return m_width;
}

auto Nedrysoft::Texture::height() const -> int {
    return m_height;
}