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

#include "Oscillator.h"

Nedrysoft::Oscillator::Oscillator() {
    initialise();

    m_settings = {
        { .frequency = 0.0078125f,    .midPoint = 16.0f },
        { .frequency = 0.0078125f,    .midPoint = 24.0f },
        { .frequency = 0.0078125f,    .midPoint = 32.0f },
        { .frequency = 0.0078125f,    .midPoint = 48.0f },
        { .frequency = 0.015625f,     .midPoint = 32.0f },
        { .frequency = 0.03125f,      .midPoint = 8.0f },
        { .frequency = 0.03125f,      .midPoint = 64.0f },
        { .frequency = 0.015625f,     .midPoint = 64.0f },
        { .frequency = 0.0078125f,    .midPoint = 80.0f },
        { .frequency = 0.0078125f,    .midPoint = 80.0f },
        { .frequency = 0.0078125f,    .midPoint = 32.0f },
        { .frequency = 0.01171875f,   .midPoint = 48.0f },
        { .frequency = 0.01953125f,   .midPoint = 80.0f },
        { .frequency = 0.02734375f,   .midPoint = 112.0f },
        { .frequency = 0.0078125f,    .midPoint = 16.0f },
        { .frequency = 0.0078125f,    .midPoint = 16.0f }
    };

    registerOscillator(32);
    registerOscillator(48);
    registerOscillator(64);
    registerOscillator(96);

    registerOscillator(64, Fast);
    registerOscillator(16);
    registerOscillator(128, Fast);
    registerOscillator(128);

    registerOscillator(160);
    registerOscillator(160, Alt);
    registerOscillator(64, Alt);
    registerOscillator(96, Alt);
    registerOscillator(160, Fast);
    registerOscillator(224);
}

auto Nedrysoft::Oscillator::getInstance() -> Oscillator * {
    static Oscillator instance;

    return &instance;
}

auto Nedrysoft::Oscillator::update(Sonic *sonic) -> void {
    if (sonic->isDying()) {
        return;
    }

    for (auto index = 0, currentBit = 0x80; index < 16; index++, currentBit >>= 1) {
        if (m_directions[index] == Down) {
            m_values[index].rate -= m_settings[index].frequency;
            m_values[index].value += m_values[index].rate;

            if (m_values[index].value <= m_settings[index].midPoint) {
                m_directions[index] = Up;
            }
        } else {
            m_values[index].rate += m_settings[index].frequency;
            m_values[index].value += m_values[index].rate;

            if (m_values[index].value >= m_settings[index].midPoint) {
                m_directions[index] = Down;
            }
        }
    }
}

auto Nedrysoft::Oscillator::initialise() -> void {
    m_values = {
        { .value = 0.5f,      .rate = 0.0f },
        { .value = 0.5f,      .rate = 0.0f },
        { .value = 0.5f,      .rate = 0.0f },
        { .value = 0.5f,      .rate = 0.0f },
        { .value = 0.5f,      .rate = 0.0f },
        { .value = 0.5f,      .rate = 0.0f },
        { .value = 0.5f,      .rate = 0.0f },
        { .value = 0.5f,      .rate = 0.0f },
        { .value = 0.5f,      .rate = 0.0f },
        { .value = 80.9375f,  .rate = 1.1171875f },
        { .value = 32.5f,     .rate = 0.703125f },
        { .value = 48.5f,     .rate = 1.0546875f },
        { .value = 80.5f,     .rate = 1.7578125f },
        { .value = 112.5,     .rate = 2.4609375f },
        { .value = 0.5f,      .rate = 0.0f },
        { .value = 0.5f,      .rate = 0.0f },
    };

    m_directions = {
        Up,
        Up,
        Down,
        Down,
        Down,
        Down,
        Down,
        Down,
        Up,
        Up,
        Up,
        Up,
        Up,
        Up,
        Up,
        Up
    };
}

void Nedrysoft::Oscillator::registerOscillator(int height, OscillatorTypes type) {
    std::string typeString;

    typeString = std::to_string(height) + "-" + std::to_string(type);

    if (m_types.count(typeString)) {
        assert(false);
    }

    m_types[typeString] = m_types.size();
}

auto Nedrysoft::Oscillator::value(int height, OscillatorTypes type, float *midPoint) -> float {
    std::string typeString;

    typeString = std::to_string(height) + "-" + std::to_string(type);

    if (!m_types.count(typeString)) {
        assert(false);
    }

    auto index = m_types[typeString];

    if (midPoint) {
        *midPoint = m_settings[index].midPoint;
    }

    return floor(m_values[index].value);
}