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

#include "DemoPlayer.h"

#include "Utils.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

Nedrysoft::DemoPlayer::DemoPlayer() :
        m_stepIndex(0) {

    reset();
}

auto Nedrysoft::DemoPlayer::load(const std::string &demoFilename) -> bool {
    reset();

    std::ifstream demoStream(demoFilename);

    nlohmann::json jsonObject = nlohmann::json::parse(demoStream, nullptr, false);

    if (jsonObject.is_discarded()) {
        std::cout << color::rize("[unable to open demo]", "Red", "Default", "Bold") +
                                 debugValue("filename", demoFilename, "Cyan", "Green", "Bold") <<
                                 std::endl;

        return false;
    }

    auto demoArray = jsonObject["demo"];

    for (auto &demoStep : demoArray) {
        DemoStep step;
        
        step.numberOfTicks = demoStep["ticks"].get<int>();

        std::string tempString;

        auto buttons = demoStep["buttons"];

        for (auto &button : buttons) {
            auto buttonString = button.get<std::string>();

            if (!tempString.empty()) {
                tempString += ", ";
            }

            tempString += buttonString;

            if (buttonString == "Right") {
                step.buttons.insert(Nedrysoft::JoystickButton::Right);
            } else if (buttonString == "Left") {
                step.buttons.insert(Nedrysoft::JoystickButton::Left);
            } if (buttonString == "Up") {
                step.buttons.insert(Nedrysoft::JoystickButton::Up);
            } if (buttonString == "Down") {
                step.buttons.insert(Nedrysoft::JoystickButton::Down);
            } if (buttonString == "A") {
                step.buttons.insert(Nedrysoft::JoystickButton::A);
            }
        }

        step.buttonsString = (tempString.empty() == false) ? tempString : "none";

        m_steps.push_back(step);
    }

    if (!m_steps.empty()) {
        m_stepCounter = m_steps[0].numberOfTicks;
    }

    return true;
}

auto Nedrysoft::DemoPlayer::reset() -> void {
    m_steps.clear();

    m_stepIndex = 0;
    m_stepCounter = 0;
}

auto Nedrysoft::DemoPlayer::update() -> bool {
    if (m_steps.empty()) {
        return true;
    }

    if (--m_stepCounter == 0xFF) {
        m_stepIndex++;

        if (m_stepIndex > m_steps.size() - 1) {
            return true;
        }

        m_stepCounter = m_steps[m_stepIndex].numberOfTicks;

        Nedrysoft::Input::getInstance()->setInputs(m_steps[m_stepIndex].buttons);

        std::cout <<  color::rize("[demo]", "Green", "Default", "Bold") +
                debugValue("button state", m_steps[m_stepIndex].buttonsString, "Cyan", "Green", "Bold") +
                debugValue("frames to next", std::to_string(m_stepCounter), "Cyan", "Green", "Bold") << 
                std::endl;
    }

    return false;
}
