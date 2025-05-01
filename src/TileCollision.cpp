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

#include "TileCollision.h"

Nedrysoft::TileCollision::TileCollision(
        std::vector<int> verticalCollisionHeights,
        std::vector<int> horizontalCollisionHeights ) {

    m_verticalCollisionHeights = std::move(verticalCollisionHeights);
    m_horizontalCollisionHeights = std::move(horizontalCollisionHeights);
}

int Nedrysoft::TileCollision::verticalHeight(int column) {
    return m_verticalCollisionHeights[column];
}

int Nedrysoft::TileCollision::horizontalHeight(int row) {
    return m_horizontalCollisionHeights[row];
}

auto Nedrysoft::TileCollision::verticalCollisions() ->  std::vector<int> {
    return m_verticalCollisionHeights;
}

auto Nedrysoft::TileCollision::horizontalCollisions() ->  std::vector<int> {
    return m_horizontalCollisionHeights;
}
