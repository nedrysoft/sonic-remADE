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

#ifndef NEDRYSOFT_INPUT_H
#define NEDRYSOFT_INPUT_H

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <map>
#include <set>

namespace Nedrysoft {

    enum class JoystickButton {
        Up = 11,
        Down = 12,
        Left = 13,
        Right = 14,

        Start = 6,
        Select = 4,

        X = 2,
        Y = 3,
        A = 0,
        B = 1,

        LeftTrigger = 9,
        RightTrigger = 10
    };

    class Input {
        public:
            static auto getInstance() -> Input *;

            auto setEnabled(JoystickButton button, bool state) -> void;

            auto handleEvent(const SDL_Event &event) -> void;

            auto pressed(JoystickButton button) -> bool;
            auto enabled(JoystickButton button) -> bool;

            auto setInputs(const std::set<JoystickButton> &buttons) -> void;

        private:
            Input();
            ~Input() = default;

        private:
            std::map<JoystickButton, bool> m_buttonState;
            std::map<JoystickButton, bool> m_buttonEnabled;

            std::map<Sint32, SDL_GameController *> m_gameControllers;
    };
}

#endif //NEDRYSOFT_INPUT_H