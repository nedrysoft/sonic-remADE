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

#include "BridgeFactory.h"

#include "Animations.h"
#include "GameRenderer.h"

Nedrysoft::Animations *Nedrysoft::BridgeFactory::m_animations = nullptr;

auto Nedrysoft::BridgeFactory::getInstance() -> BridgeFactory * {
    static Nedrysoft::BridgeFactory instance;

    return &instance;
}

auto Nedrysoft::BridgeFactory::initialise() -> BridgeFactory * {
    Nedrysoft::Bridge::initialise();

    return getInstance();
}

auto Nedrysoft::BridgeFactory::update() -> void {
    Nedrysoft::Bridge::update();
}

auto Nedrysoft::BridgeFactory::create(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) -> Bridge * {
    return new Bridge(subType, x, y, rememberState, mirrored, flipped);
}