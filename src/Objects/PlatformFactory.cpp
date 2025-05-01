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

#include "PlatformFactory.h"

#include "Animations.h"
#include "GameRenderer.h"

Nedrysoft::Animations *Nedrysoft::PlatformFactory::m_animations = nullptr;

auto Nedrysoft::PlatformFactory::getInstance() -> PlatformFactory * {
    static Nedrysoft::PlatformFactory instance;

    return &instance;
}

auto Nedrysoft::PlatformFactory::initialise() -> PlatformFactory * {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_animations = Animations::load(gameRenderer->renderer(), "./data/art/objects/ghz-platforms/ghz-platforms.json");

    return getInstance();
}

auto Nedrysoft::PlatformFactory::update() -> void {
    Nedrysoft::Platform::update();
}

auto Nedrysoft::PlatformFactory::create(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) -> Platform * {
    return new Platform(subType, x, y, rememberState, mirrored, flipped);
}

auto Nedrysoft::PlatformFactory::animations() -> Nedrysoft::Animations * {
    return m_animations;
}