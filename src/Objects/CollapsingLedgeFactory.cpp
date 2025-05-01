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

#include "CollapsingLedgeFactory.h"

Nedrysoft::Animations *Nedrysoft::CollapsingLedgeFactory::m_animations = nullptr;

auto Nedrysoft::CollapsingLedgeFactory::getInstance() -> CollapsingLedgeFactory * {
    static Nedrysoft::CollapsingLedgeFactory instance;

    return &instance;
}

auto Nedrysoft::CollapsingLedgeFactory::initialise() -> CollapsingLedgeFactory * {
    Nedrysoft::CollapsingLedge::initialise();

    return getInstance();
}

auto Nedrysoft::CollapsingLedgeFactory::update() -> void {
    Nedrysoft::CollapsingLedge::update();
}

auto Nedrysoft::CollapsingLedgeFactory::create(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) -> CollapsingLedge * {
    return new CollapsingLedge(subType, x, y, rememberState, mirrored, flipped);
}