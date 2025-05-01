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

#include "LamppostFactory.h"

Nedrysoft::Animations *Nedrysoft::LamppostFactory::m_animations = nullptr;

auto Nedrysoft::LamppostFactory::getInstance() -> LamppostFactory * {
    static Nedrysoft::LamppostFactory instance;

    return &instance;
}

auto Nedrysoft::LamppostFactory::initialise() -> LamppostFactory * {
    Nedrysoft::Lamppost::initialise();

    return getInstance();
}

auto Nedrysoft::LamppostFactory::update() -> void {
    Nedrysoft::Lamppost::update();
}

auto Nedrysoft::LamppostFactory::create(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) -> Lamppost * {
    return new Lamppost(subType, x, y, rememberState, mirrored, flipped);
}
