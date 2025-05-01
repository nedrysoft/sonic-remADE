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

#include "PurpleRockFactory.h"

#include "Animations.h"
#include "GameRenderer.h"

Nedrysoft::Animations *Nedrysoft::PurpleRockFactory::m_animations = nullptr;

auto Nedrysoft::PurpleRockFactory::getInstance() -> PurpleRockFactory * {
    static Nedrysoft::PurpleRockFactory instance;

    return &instance;
}

auto Nedrysoft::PurpleRockFactory::initialise() -> PurpleRockFactory * {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_animations = Animations::load(gameRenderer->renderer(), "./data/art/objects/ghz-purple-rock/ghz-purple-rock.json");

    return getInstance();
}

auto Nedrysoft::PurpleRockFactory::update() -> void {
    Nedrysoft::PurpleRock::update();
}

auto Nedrysoft::PurpleRockFactory::create(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) -> PurpleRock * {
    return new PurpleRock(subType, x, y, rememberState, mirrored, flipped);
}

auto Nedrysoft::PurpleRockFactory::animations() -> Nedrysoft::Animations * {
    return m_animations;
}