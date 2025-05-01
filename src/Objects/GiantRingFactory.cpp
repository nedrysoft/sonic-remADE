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

#include "GiantRingFactory.h"

#include "Animations.h"
#include "GameRenderer.h"

Nedrysoft::Animations *Nedrysoft::GiantRingFactory::m_animations = nullptr;

auto Nedrysoft::GiantRingFactory::getInstance() -> GiantRingFactory * {
    static Nedrysoft::GiantRingFactory instance;

    return &instance;
}

auto Nedrysoft::GiantRingFactory::initialise() -> GiantRingFactory * {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_animations = Animations::load(gameRenderer->renderer(), "./data/art/objects/giant-ring/giant-ring.json");

    Nedrysoft::GiantRing::initialise();

    return getInstance();
}

auto Nedrysoft::GiantRingFactory::update() -> void {
    Nedrysoft::GiantRing::update();
}

auto Nedrysoft::GiantRingFactory::create(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) -> GiantRing * {
    return new GiantRing(subType, x, y, rememberState, mirrored, flipped);
}

auto Nedrysoft::GiantRingFactory::animations() -> Nedrysoft::Animations * {
    return m_animations;
}
