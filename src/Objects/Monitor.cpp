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

#include "Monitor.h"

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "Explosion.h"
#include "GameRenderer.h"
#include "Hud.h"
#include "Input.h"
#include "MonitorFactory.h"
#include "Object.h"
#include "ObjectsManager.h"
#include "Points.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>

constexpr auto GravityCoefficient = 0.21875;
constexpr auto MaximumPushForce = 2.25f;

enum MonitorCollisionState {
    NoCollision = 0,
    CollisionOnLeft = 1,
    CollisionOnRight = 2,
    CollisionInX = 4,
    CollisionInY = 8
};

auto Nedrysoft::Monitor::initialise() -> void {

}

Nedrysoft::Monitor::Monitor(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Solid("monitor", subType, x, y, rememberState, mirrored, flipped) {

    reset();
}

auto Nedrysoft::Monitor::spawn(Camera *camera) -> bool {
    std::string animationName;

    if (m_state == MonitorState::Collected) {
        animationName = "broken";
    } else {
        switch(m_subType) {
            case Static: {
                animationName = "static";
                break;
            }
            case Eggman: {
                animationName = "eggman";
                break;
            }
            case Sonic: {
                animationName = "sonic";
                break;
            }
            case Shoes: {
                animationName = "shoes";
                break;
            }
            case Shield: {
                animationName = "shield";
                break;
            }
            case Invincible: {
                animationName = "invincible";
                break;
            }
            case Rings: {
                animationName = "rings";
                break;
            }
            case S: {
                animationName = "s";
                break;
            }
            case Goggles: {
                animationName = "goggles";
                break;
            }

            default: {
                assert(false);
            }
        }
    }

    auto animations = Nedrysoft::MonitorFactory::getInstance()->animations();

    assert(animations);

    animations->start(animationName, &m_animation);

    int width;
    int height;

    Vector origin;

    m_animation.texture(&width, &height, &origin);

    m_collisionRect.x = static_cast<int>(m_x - origin.x());
    m_collisionRect.w = width;
    m_collisionRect.y = static_cast<int>(m_y - origin.y());
    m_collisionRect.h = height;

    return true;
}

auto Nedrysoft::Monitor::checkForCollisionAction(float xDistance, float yDistance, TileMap *tileMap, Camera *camera,
                                                 class Sonic *sonic) -> bool {
    N_UNUSED(yDistance)

    if (sonic->isJumpingUp()) {
        if (sonic->rect().top() > m_y) {
            sonic->setObjectCollidedTop(0);

            m_ySpeed -= std::min(MaximumPushForce, fabs(sonic->verticalSpeed()));

            m_collisionFlags = Bottom;

            return true;
        }
    } else if (sonic->isJumpingDown()) {
        auto animations = Nedrysoft::MonitorFactory::getInstance()->animations();

        assert(animations);

        animations->start("breaking", &m_animation);

//TODO: Check this behaviour, I disabled.
        //sonic->setStandingOnObject(nullptr);

        m_state = Nedrysoft::Collected;

        sonic->rebound(Sonic::ReboundMode::Monitor, Vector(m_x, m_y));

        Nedrysoft::Audio::getInstance()->playSample(1, Nedrysoft::SoundId::Break);

        setCollisionEnabled(false);

        sonic->addPoints(100);

        auto objectsManager = Nedrysoft::ObjectsManager::getInstance();

        objectsManager->addDynamicObject(new Points(100, m_x, m_y));
        objectsManager->addDynamicObject(new Nedrysoft::Explosion(m_x, m_y));

        return true;
    }

    return false;
}

auto Nedrysoft::Monitor::objectUpdating(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> void {
    N_UNUSED(camera)
    N_UNUSED(sonic)

    if (m_ySpeed) {
        int width, height;
        Vector origin;
        float distance;

        m_animation.texture(&width, &height, &origin);

        m_collisionRect.x = static_cast<int>(m_x - origin.x());
        m_collisionRect.w = width;
        m_collisionRect.y = static_cast<int>(m_y - origin.y());
        m_collisionRect.h = height;

        Vector sensorPosition(m_x, (m_y + height / 2.0f));

        if (m_ySpeed > 0) {
            if (tileMap->findDown(sensorPosition, &distance)) {

                if (distance <= 0) {
                    m_y += distance;

                    m_ySpeed = 0;

                    return;
                }
            }
        }

        m_ySpeed += GravityCoefficient;

        m_y += m_ySpeed;
    }
}

auto Nedrysoft::Monitor::reset() -> void {
    Solid::reset();

    m_state = MonitorState::Uncollected;
}  