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

#include "Tile.h"

auto Nedrysoft::Tile::setAngle(float angle) -> void {
    m_angle = angle;
}

auto Nedrysoft::Tile::angle() const -> float {
    return m_angle;
}

auto Nedrysoft::Tile::tileIndex() const -> int {
    return m_tileIndex;
}

auto Nedrysoft::Tile::setTileIndex(int tileIndex) -> void {
    m_tileIndex = tileIndex;
}

auto Nedrysoft::Tile::flags() -> TileFlag & {
    return m_flags;
}

auto Nedrysoft::Tile::setFlags(const TileFlag &flags) -> void {
    m_flags = flags;
}

auto Nedrysoft::Tile::setChunkIndex(int chunkIndex) -> void {
    m_chunkIndex = chunkIndex;
}

auto Nedrysoft::Tile::chunkIndex() const -> int {
    return m_chunkIndex;
}

auto Nedrysoft::Tile::setOriginalTileIndex(int tileIndex) -> void {
    m_originalTileIndex = tileIndex;
}

auto Nedrysoft::Tile::setOriginalAngle(uint8_t angle) -> void {
    m_originalAngle = angle;
}

auto Nedrysoft::Tile::setOriginalSolid(uint8_t solid) -> void {
    m_originalSolid = solid;
}

auto Nedrysoft::Tile::originalAngle() const -> uint8_t {
    return m_angle;
}

auto Nedrysoft::Tile::originalTileIndex() const -> int {
    return m_tileIndex;
}

auto Nedrysoft::Tile::originalSolid() const -> uint8_t {
    return m_originalSolid;
}