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

#include "PaletteCycle.h"

#include <utility>

Nedrysoft::PaletteCycleData::PaletteCycleData(int startIndex, std::vector<uint32_t> colours) :
        m_startIndex(startIndex),
        m_colours(std::move(colours)) {

}

auto Nedrysoft::PaletteCycleData::startIndex() const -> int {
    return m_startIndex;
}

auto Nedrysoft::PaletteCycleData::colours() const -> std::vector<uint32_t> * {
    return const_cast<std::vector<uint32_t> *>(&m_colours);
}

Nedrysoft::PaletteCycle::PaletteCycle(std::string name, int frames, std::vector<PaletteCycleData> data) :
        m_name(std::move(name)),
        m_frames(frames),
        m_currentFrame(frames),
        m_data(std::move(data)),
        m_currentIndex(0) {

}

auto Nedrysoft::PaletteCycle::needsUpdate() -> bool {
    m_currentFrame--;

    if (m_currentFrame > 0) {
        return false;
    }

    m_currentFrame = m_frames;

    return true;
}

auto Nedrysoft::PaletteCycle::update(uint32_t *palette) -> bool {
    if (!needsUpdate()) {
        return false;
    }
    int index = 0;

    for(auto colour : *m_data[m_currentIndex].colours()) {
        palette[m_data[m_currentIndex].startIndex() + (index++)] = colour;
    }

    m_currentIndex = (m_currentIndex + 1) % static_cast<int>(m_data.size());

    return true;
}

auto Nedrysoft::PaletteCycle::name() -> std::string {
    return m_name;
}