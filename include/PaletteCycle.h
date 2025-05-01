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

#ifndef NEDRYSOFT_PALETTECYCLE_H
#define NEDRYSOFT_PALETTECYCLE_H

#include <string>
#include <vector>

namespace Nedrysoft {
    class PaletteCycleData {
        public:
            PaletteCycleData(int startIndex, std::vector<uint32_t> colours);

            [[nodiscard]] auto startIndex() const -> int;
            [[nodiscard]] auto colours() const -> std::vector<uint32_t> *;

        private:
            int m_startIndex;
            std::vector<uint32_t> m_colours;
    };

    class PaletteCycle {
        public:
            PaletteCycle(std::string name, int frames, std::vector<PaletteCycleData> data);

            auto update(uint32_t *palette) -> bool;

            auto name() -> std::string;

        private:
            auto needsUpdate() -> bool;

        private:
            std::string m_name;

            int m_frames;
            int m_currentFrame;
            int m_currentIndex;

            std::vector<PaletteCycleData> m_data;
    };
}

#endif //NEDRYSOFT_PALETTECYCLE_H
