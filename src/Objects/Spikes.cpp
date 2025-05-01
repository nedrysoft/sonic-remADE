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

#include "Spikes.h"

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Input.h"
#include "Object.h"
#include "SpikesFactory.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>

enum SpikeTypes {
    OneHorizontal = 0x50,
    OneVertical = 0x20,
    ThreeHorizontal = 0x10,
    ThreeVertical  = 0x00,
    ThreeWideVertical = 0x30,
    SixWideVertical = 0x40,

    Stationary = 0x00,
    VerticalMovement = 0x01,
    HorizontalMovement = 0x02
};

Nedrysoft::Spikes::Spikes(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Solid("spikes", subType, x, y, rememberState, mirrored, flipped) {

}

auto Nedrysoft::Spikes::spawn(Camera *camera) -> bool {
    auto animations = Nedrysoft::SpikesFactory::getInstance()->animations();
    std::string animationName;

    assert(animations);

    switch(m_subType & 0xF0) {
        case SpikeTypes::OneVertical: {
            animationName = "1up";

            break;
        }

        case SpikeTypes::ThreeVertical: {
            animationName = "3up";

            break;
        }

        case SpikeTypes::ThreeWideVertical: {
            animationName = "3upwide";

            break;
        }

        case SpikeTypes::SixWideVertical: {
            animationName = "6upwide";

            break;
        }
    }

    assert(!animationName.empty());

    animations->start(animationName, &m_animation);

    createHitBox(&m_animation);

    return true;
}

auto Nedrysoft::Spikes::checkForCollisionAction(
        float xDistance,
        float yDistance,
        TileMap *tileMap,
        Camera *camera,
        class Sonic *sonic ) -> bool {

    N_UNUSED(yDistance)
    N_UNUSED(tileMap)
    N_UNUSED(camera)

    if (yDistance >= 0) {
        return false;
    }

    if (xDistance > yDistance) {
        return false;
    }

    hurtSonic(yDistance, sonic, true);

    return true;
}

auto Nedrysoft::Spikes::reset() -> void {
    Object::reset();
}  