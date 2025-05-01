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

#ifndef NEDRYSOFT_TILE_H
#define NEDRYSOFT_TILE_H

#include <stdint.h>

namespace Nedrysoft {
    enum class TileFlag : int {
        None = 0,

        TopSolid = 1,
        LeftSolid = 2,
        RightSolid = 4,
        BottomSolid = 8,

        LeftRightBottomSolid = LeftSolid | RightSolid | BottomSolid,

        AllSolid = TopSolid | LeftRightBottomSolid,

        CalculatedAngle = 16,

        TopLeftPriority = 32,
        TopRightPriority = 64,
        BottomLeftPriority = 128,
        BottomRightPriority = 256,

        TopPriority = TopLeftPriority | TopRightPriority,
        BottomPriority = BottomLeftPriority | BottomRightPriority,
        LeftPriority = TopLeftPriority | BottomLeftPriority,
        RightPriority = TopRightPriority | BottomRightPriority,

        AllPriority = TopLeftPriority | BottomLeftPriority | TopRightPriority | BottomRightPriority,
    };

    inline constexpr TileFlag operator & (TileFlag x, TileFlag y) {
        return static_cast<TileFlag>(static_cast<int>(x) & static_cast<int>(y));
    }

    inline constexpr TileFlag operator | (TileFlag x, TileFlag y) {
        return static_cast<TileFlag>(static_cast<int>(x) | static_cast<int>(y));
    }

    inline constexpr TileFlag & operator |= (TileFlag & x, TileFlag y) {
        x = x | y;

        return x;
    }

    inline constexpr TileFlag operator ^ (TileFlag x, TileFlag y) {
        return static_cast<TileFlag>(static_cast<int>(x) ^ static_cast<int>(y));
    }

    inline constexpr TileFlag operator ~ (TileFlag x) {
        return static_cast<TileFlag>(~static_cast<int>(x));
    }

    class Tile {
        public:

            [[nodiscard]] auto angle() const -> float;
            [[nodiscard]] auto tileIndex() const -> int;
            [[nodiscard]] auto originalAngle() const -> uint8_t;
            [[nodiscard]] auto originalTileIndex() const -> int;
            [[nodiscard]] auto originalSolid() const -> uint8_t;

            auto setAngle(float angle) -> void;
            auto setTileIndex(int tileIndex) -> void;
            auto flags() -> TileFlag &;
            auto setFlags(const TileFlag &flags) -> void;
            auto setChunkIndex(int chunkIndex) -> void;
            auto setOriginalTileIndex(int tileIndex) -> void;
            auto setOriginalAngle(uint8_t angle) -> void;
            auto setOriginalSolid(uint8_t solidIndex) -> void;
            [[nodiscard]] auto chunkIndex() const -> int;

        private:
            float m_angle;
            int m_tileIndex;
            int m_chunkIndex;
            int m_originalTileIndex;
            uint8_t m_originalAngle;
            uint8_t m_originalSolid;

            TileFlag m_flags;
    };
}

#endif //NEDRYSOFT_TILE_H
