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

#include "Animation.h"
#include "AnimationStep.h"

Nedrysoft::Animation::Animation(std::string name, int speed, std::vector<AnimationStep *> script) :
        m_name(std::move(name)),
        m_speed(speed),
        m_script(std::move(script)) {

}

Nedrysoft::Animation::Animation() :
        m_name(std::string()),
        m_speed(60),
        m_script(std::vector<AnimationStep *>()) {

}

auto Nedrysoft::Animation::name() -> std::string {
    return m_name;
}

[[nodiscard]] auto Nedrysoft::Animation::speed() const -> int {
    return m_speed;
}

auto Nedrysoft::Animation::script() -> std::vector<AnimationStep *> & {
    return m_script;
}

auto Nedrysoft::Animation::setName(std::string name) -> void {
    m_name = std::move(name);
}

auto Nedrysoft::Animation::setSpeed(int speed) -> void {
    m_speed = speed;
}

auto Nedrysoft::Animation::setScript(std::vector<AnimationStep *> script) -> void {
    m_script = std::move(script);
}

auto Nedrysoft::Animation::start() -> AnimationStep * {
    if (!m_script.empty()) {
        for (auto step : m_script) {
            if (step->type() == AnimationStepType::Show) {
                return step;
            }
        }
    }

    return nullptr;
}
