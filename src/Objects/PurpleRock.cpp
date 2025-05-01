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

#include "PurpleRock.h"

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Input.h"
#include "Object.h"
#include "PurpleRockFactory.h"
#include "Sonic.h"

#include <SDL2/SDL.h>

Nedrysoft::PurpleRock::PurpleRock(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Solid("purple-rock", subType, x, y, rememberState, mirrored, flipped) {

}

auto Nedrysoft::PurpleRock::spawn(Camera *camera) -> bool {
    auto animations = Nedrysoft::PurpleRockFactory::getInstance()->animations();

    assert(animations);

    animations->start("purple-rock", &m_animation);

    int width = (static_cast<int>(m_animation.width()) - 32) - 2;

    createHitBox(&m_animation, width / 2, 0, -width);

    return true;
}

auto Nedrysoft::PurpleRock::reset() -> void {
    Solid::reset();
}  

