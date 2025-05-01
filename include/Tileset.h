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

#ifndef NEDRYSOFT_TILESET_H
#define NEDRYSOFT_TILESET_H

#include "Texture.h"
#include "TileCollision.h"

#include <Magick++.h>
#include <vector>

namespace Nedrysoft {
    class Tileset {
        public:
            Tileset();

            auto setSize(int width, int height) -> void;

            [[nodiscard]] auto tileWidth() const -> int;
            [[nodiscard]] auto tileHeight() const -> int;

            auto setTileImage(const Magick::Image &image) -> void;
            auto tileImage() -> Magick::Image &;

            auto collisions() -> std::vector<TileCollision *> *;

            auto tiles() -> std::vector<Texture *> *;
            auto solids() -> std::vector<Texture *> *;

            auto setTileWidth(int width) -> void;
            auto setTileHeight(int height) -> void;

        private:
            int m_tileWidth;
            int m_tileHeight;

            Magick::Image m_tileImage;

            std::vector<Texture *> m_tiles;
            std::vector<Texture *> m_solids;

            std::vector<TileCollision *> m_collisions;
    };
}

#endif //NEDRYSOFT_TILESET_H
