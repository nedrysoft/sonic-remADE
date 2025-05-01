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

#include "Input.h"

#include "DebugManager.h"

Nedrysoft::Input::Input() = default;

auto Nedrysoft::Input::getInstance() -> Input * {
    static Input instance;

    return &instance;
}

auto Nedrysoft::Input::setEnabled(JoystickButton button, bool state) -> void {
    if (m_buttonState.count(button) == 0) {
        m_buttonState[button] = false;
        m_buttonEnabled[button] = state;
    }

    m_buttonEnabled[button] = state;
}

auto Nedrysoft::Input::setInputs(const std::set<JoystickButton> &buttons) -> void {
    std::set<JoystickButton> availableButtons = {
        Nedrysoft::JoystickButton::Up,
        Nedrysoft::JoystickButton::Down,
        Nedrysoft::JoystickButton::Left,
        Nedrysoft::JoystickButton::Right,
        Nedrysoft::JoystickButton::Start,
        Nedrysoft::JoystickButton::Select,
        Nedrysoft::JoystickButton::X,
        Nedrysoft::JoystickButton::Y,
        Nedrysoft::JoystickButton::A,
        Nedrysoft::JoystickButton::B,
        Nedrysoft::JoystickButton::LeftTrigger,
        Nedrysoft::JoystickButton::RightTrigger
    };

    for (auto &button : availableButtons) {
        if (buttons.count(button)) {
            m_buttonState[button] = true;
        } else {
            m_buttonState[button] = false;
        }
    }
}

auto Nedrysoft::Input::pressed(JoystickButton button) -> bool {
    if (m_buttonState.count(button) == 0) {
        m_buttonState[button] = false;
        m_buttonEnabled[button] = true;
    }

    return m_buttonState[button];
}

auto Nedrysoft::Input::enabled(JoystickButton button) -> bool {
    if (m_buttonState.count(button) == 0) {
        m_buttonState[button] = false;
        m_buttonEnabled[button] = true;
    }

    return m_buttonEnabled[button];
}

auto Nedrysoft::Input::handleEvent(const SDL_Event &event) -> void {
    switch(event.type) {
        case SDL_JOYDEVICEADDED: {
            m_gameControllers[event.cdevice.which] = SDL_GameControllerOpen(event.cdevice.which);

            DebugManager::getInstance()->addText("inputJoyConnect", "Game Controller Connected");

            break;
        }

        case SDL_JOYDEVICEREMOVED: {
            SDL_GameControllerClose(m_gameControllers[event.cdevice.which]);

            DebugManager::getInstance()->addText("inputJoyConnect", "Game Controller Disconnected");

            m_gameControllers.erase(event.cdevice.which);

            for (auto &state : m_buttonState) {
                state.second = false;
            }
        }

        case SDL_JOYBUTTONDOWN: {
            m_buttonState[static_cast<JoystickButton>(event.jbutton.button)] = true;

            break;
        }
        case SDL_JOYBUTTONUP: {
            m_buttonState[static_cast<JoystickButton>(event.jbutton.button)] = false;

            break;
        }
    }
}
