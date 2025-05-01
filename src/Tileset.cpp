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

#include "Tileset.h"

Nedrysoft::Tileset::Tileset() :
        m_tileWidth(0),
        m_tileHeight(0) {

}

auto Nedrysoft::Tileset::setSize(int width, int height) -> void {
    m_tileWidth = width;
    m_tileHeight = height;
}

auto Nedrysoft::Tileset::tileWidth() const -> int {
    return m_tileWidth;
}

auto Nedrysoft::Tileset::tileHeight() const -> int {
    return m_tileHeight;
}

auto Nedrysoft::Tileset::setTileImage(const Magick::Image &image) -> void {
    m_tileImage = image;
}

auto Nedrysoft::Tileset::tileImage() -> Magick::Image & {
    return m_tileImage;
}

auto Nedrysoft::Tileset::collisions() -> std::vector<TileCollision *> * {
    return &m_collisions;
}

auto Nedrysoft::Tileset::tiles() -> std::vector<Texture *> * {
    return &m_tiles;
}

auto Nedrysoft::Tileset::solids() -> std::vector<Texture *> * {
    return &m_solids;
}

auto Nedrysoft::Tileset::setTileWidth(int width) -> void {
    m_tileWidth = width;
}

auto Nedrysoft::Tileset::setTileHeight(int height) -> void {
    m_tileHeight = height;
}
