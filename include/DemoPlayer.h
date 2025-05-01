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

#ifndef NEDRYSOFT_DEMOPLAYER_H
#define NEDRYSOFT_DEMOPLAYER_H

#include "Input.h"

#include <set>
#include <string>
#include <vector>

namespace Nedrysoft {;
    class DemoPlayer {
        private:
            struct DemoStep {
                int numberOfTicks;
                std::set<Nedrysoft::JoystickButton> buttons;
                std::string buttonsString;
            };

        public:
            DemoPlayer();

            auto load(const std::string &demoFilename) -> bool;

            auto reset() -> void;
            auto update() -> bool;

        private:
            std::vector<DemoStep> m_steps;

            int m_stepIndex;
            uint8_t m_stepCounter;
    };
}

#endif //NEDRYSOFT_DEMOPLAYER_H
