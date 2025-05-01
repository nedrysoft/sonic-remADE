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

#include "EdgeWall.h"

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "EdgeWallFactory.h"
#include "GameRenderer.h"
#include "Input.h"
#include "Object.h"
#include "Sonic.h"

#include <SDL2/SDL.h>

enum WallType {
    Shadowed = 0,
    Light = 1,
    Dark = 2
};

Nedrysoft::EdgeWall::EdgeWall(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Solid("edge-wall", subType, x, y, rememberState, mirrored, flipped) {

}

auto Nedrysoft::EdgeWall::spawn(Camera *camera) -> bool {
    auto animations = Nedrysoft::EdgeWallFactory::getInstance()->animations();

    assert(animations);

    switch(m_subType) {
        case WallType::Shadowed: {
            animations->start("shadow", &m_animation);

            break;
        }

        case WallType::Light: {
            animations->start("light", &m_animation);

            break;
        }

        case WallType::Dark: {
            animations->start("dark", &m_animation);

            break;
        }

        default: {
            assert(false);
        }
    }

    createHitBox(&m_animation);

    return true;
}

auto Nedrysoft::EdgeWall::reset() -> void {
    Solid::reset();
}  