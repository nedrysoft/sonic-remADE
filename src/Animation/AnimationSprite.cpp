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

#include "AnimationSprite.h"

Nedrysoft::AnimationSprite::AnimationSprite(std::string name, Vector origin, int width, int height, SDL_Texture *texture) :
        m_name(std::move(name)),
        m_origin(origin),
        m_width(width),
        m_height(height),
        m_texture(texture) {

}

auto Nedrysoft::AnimationSprite::origin() -> Vector {
    return m_origin;
}

auto Nedrysoft::AnimationSprite::width() const -> int {
    return m_width;
}

auto Nedrysoft::AnimationSprite::height() const -> int {
    return m_height;
}

auto Nedrysoft::AnimationSprite::texture() -> SDL_Texture * {
    return m_texture;
}

auto Nedrysoft::AnimationSprite::name() -> std::string {
    return m_name;
}
