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

#ifndef NEDRYSOFT_TILECOLLISION_H
#define NEDRYSOFT_TILECOLLISION_H

#include <vector>

namespace Nedrysoft {
    class TileCollision {
        public:
            TileCollision(std::vector<int> verticalCollisionHeights, std::vector<int> horizontalCollisionHeights);

            auto verticalHeight(int column) -> int;

            auto horizontalHeight(int row) -> int;

            auto verticalCollisions() ->  std::vector<int>;
            auto horizontalCollisions() ->  std::vector<int>;

        private:
            std::vector<int> m_verticalCollisionHeights;
            std::vector<int> m_horizontalCollisionHeights;
    };
}

#endif //NEDRYSOFT_TILECOLLISION_H
