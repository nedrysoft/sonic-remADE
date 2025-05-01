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

#include "Spring.h"

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Input.h"
#include "Object.h"
#include "SpringFactory.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>

auto constexpr CollisionWidthRadius = 16;
auto constexpr CollisionHeightRadius = 8;
auto constexpr ControlLockTimer = 16;

enum SpringTypes {
    Red = 0x00,
    Yellow = 0x02,

    Vertical = 0x00,
    Horizontal = 0x10
};

Nedrysoft::Spring::Spring(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Solid("spring", subType, x, y, rememberState, mirrored, flipped) {

    m_collisionFlags = Solidity::Top;
}

auto Nedrysoft::Spring::spawn(Camera *camera) -> bool {
    auto animations = Nedrysoft::SpringFactory::getInstance()->animations();

    assert(animations);

    std::string colour;
    std::string orientation;

    switch(m_subType & 0x0F) {
        case SpringTypes::Red: {
            colour = "red";

            break;
        }

        case SpringTypes::Yellow: {
            colour = "yellow";

            break;
        }

        default: {
            assert(false);
        }
    }

    switch(m_subType & 0xF0) {
        case SpringTypes::Vertical: {
            orientation = "up";

            break;
        }

        case SpringTypes::Horizontal: {
            orientation = "left";

            break;
        }

        default: {
            assert(false);
        }
    }

    animations->start(colour + "-" + orientation, &m_animation);

    m_collisionRect.x = static_cast<int>(m_x)- CollisionWidthRadius;
    m_collisionRect.y = static_cast<int>(m_y) - CollisionHeightRadius;
    m_collisionRect.w = CollisionWidthRadius * 2;
    m_collisionRect.h = CollisionHeightRadius * 2;

    return true;
}

auto Nedrysoft::Spring::checkForCollisionAction(
        float xDistance,
        float yDistance,
        TileMap *tileMap,
        Camera *camera,
        class Sonic *sonic ) -> bool {

    N_UNUSED(yDistance)
    N_UNUSED(tileMap)
    N_UNUSED(camera)

    float speed;
    float xSpeed = sonic->horizontalSpeed();
    float ySpeed = sonic->verticalSpeed();

    if (sonic->onGround() && (sonic->rect().y() + sonic->rect().height() > m_y)) {
        return false;
    }

    if (yDistance > 0) {
        return false;
    }

    auto animations = Nedrysoft::SpringFactory::getInstance()->animations();

    assert(animations);

    std::string colour;
    std::string orientation;

    switch(m_subType & 0x0F) {
        case SpringTypes::Red: {
            colour = "red";

            speed = 16.0f;

            break;
        }

        case SpringTypes::Yellow: {
            colour = "yellow";

            speed = 10.0f;

            break;
        }

        default: {
            assert(false);
        }
    }

    switch(m_subType & 0xF0) {
        case SpringTypes::Vertical: {
            orientation = "up";

            if (!m_mirrored) {
                ySpeed = -speed;
                yDistance += 8.0f;
            } else {
                ySpeed = speed;
                yDistance -= 8.0f;
            }

            //xSpeed = 0.0f;
            xDistance = 0;

            break;
        }

        case SpringTypes::Horizontal: {
            orientation = "left";

            if (!m_mirrored) {
                xSpeed = -speed;
                xDistance -= 8.0f;
            } else {
                xSpeed = speed;
                xDistance += 8.0f;
            }

            //ySpeed = 0.0f;
            yDistance = 0;

            break;
        }

        default: {
            assert(false);
        }
    }

    sonic->rocket(xDistance, yDistance, xSpeed, ySpeed, ControlLockTimer);

    animations->start(colour + "-" + orientation + "-activated" , &m_animation);

    Nedrysoft::Audio::getInstance()->playSample(1, Nedrysoft::SoundId::Spring);

    return true;
}

auto Nedrysoft::Spring::reset() -> void {
    Solid::reset();
}  