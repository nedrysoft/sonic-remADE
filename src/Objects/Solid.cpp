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

#include "Solid.h"

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Input.h"
#include "Object.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>

#include <utility>

//#define DEBUG_DRAW_COLLISION_BOX

Nedrysoft::Solid::Solid(std::string type, int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Object(std::move(type), subType, x, y, rememberState, mirrored, flipped),
        m_collisionRect {.x = 0, .y = 0, .w = 0, .h = 0},
        m_animation(Nedrysoft::AnimationPlayer()),
        m_solidity(Top | LeftRightBottom) {

    m_ySpeed = 0;
    m_collisionFlags = None;
    m_collisionEnabled = true;
}

auto Nedrysoft::Solid::update() -> void {

}

auto Nedrysoft::Solid::initialise() -> void {

}

auto Nedrysoft::Solid::update(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    SDL_Rect sonicRect;
    SDL_Rect resultRect;

    m_animation.next();

    if (!m_collisionEnabled) {
        return false;
    }

    objectUpdating(tileMap, camera, sonic);

    /**
     * if sonic is dying then he can't collide with anything, so we exit early.
     */

    if (sonic->isDying()) {
        return false;
    }

    auto leftButton = Nedrysoft::Input::getInstance()->pressed(JoystickButton::Left);
    auto rightButton = Nedrysoft::Input::getInstance()->pressed(JoystickButton::Right);

    /**
     * now we check to see if sonic is colliding with the monitor.
     */

    sonicRect = sonic->hitBox();

    if (!SDL_IntersectRect(&m_collisionRect, &sonicRect, &resultRect)) {
        if (sonic->standingOnObject() == this) {
            sonic->setStandingOnObject(nullptr);
        }

        m_collisionFlags = 0;

        return false;
    }

    /**
     * we know that sonic is inside the hit box, so now we need to figure out where the collision is actually
     * occurring so that we can deal with sonics interaction with the object, first we figure out
     */

    auto hitboxWidthRadius = static_cast<int>(m_animation.width() / 2.0f);

    int yDistance = m_collisionRect.y - static_cast<int>(sonic->rect().bottom());
    int xDistance = 0;

    int collisionState = SolidCollisionStates::NoCollision;

    if (sonic->hitBox().x < static_cast<int>(m_x)) {
        xDistance = static_cast<int>(m_x) - hitboxWidthRadius - (sonic->hitBox().x + sonic->hitBox().w);

        collisionState |= SolidCollisionStates::CollisionOnLeft;
    } else if (sonic->hitBox().x > static_cast<int>(m_x)) {
        xDistance  = (sonic->hitBox().x) - (static_cast<int>(m_x) + hitboxWidthRadius);

        collisionState |= SolidCollisionStates::CollisionOnRight;
    }

    /**
     * we check the previous collision state first, since we can then many edge cass based on the previous state.
     */

    switch(m_collisionFlags) {
        case SolidCollisionFlags::Top: {
            if (sonic->standingOnObject()) {
                setStandingOnObject(yDistance, tileMap, camera, sonic);

                return false;
            }

            break;
        }

        case SolidCollisionFlags::Bottom: {
            break;
        }

        case SolidCollisionFlags::Left: {
            if (rightButton) {
                sonic->setObjectCollidedRight(m_collisionRect.x - (sonic->hitBox().w / 2), sonic->onGround());

                return false;
            }

            break;
        }

        case SolidCollisionFlags::Right: {
            if (leftButton) {
                sonic->setObjectCollidedLeft(m_collisionRect.x + m_collisionRect.w + (sonic->hitBox().w / 2), sonic->onGround());

                return false;
            }
            break;
        }

        default: {
            break;
        }
    }

    /**
     * we know there is a collision, do we perform an initial collision check using the checkForCollisionAction which
     * may have been overridden by Solid subclasses, if a subclass has overridden this function then it may handle
     * the collision completely, in which case, we just exit.
     *
     * If it doesn't handle the collision (or it decided not to process it), then the default processing continues,
     * we will check if the collision was in X or Y and process accordingly.
     */

    if (checkForCollisionAction(xDistance, yDistance, tileMap, camera, sonic)) {
        return false;
    }

    /**
     * we determine which axis the primary collision occurred in.
     */

    if ((xDistance < 0) && (yDistance < 0)) {
        if ((m_solidity & LeftRightBottom) && (xDistance > yDistance)) {
            collisionState |= SolidCollisionStates::CollisionInX;
        } else {
            collisionState |= SolidCollisionStates::CollisionInY;
        }
    }

    if (!sonic->standingOnObject()) {
        if (collisionState & SolidCollisionStates::CollisionInX) {
            if (collisionState & SolidCollisionStates::CollisionOnLeft) {
                m_collisionFlags = SolidCollisionFlags::Left;

                sonic->setObjectCollidedRight(m_collisionRect.x - (sonic->hitBox().w / 2), sonic->onGround());
            } else {
                m_collisionFlags = SolidCollisionFlags::Right;

                sonic->setObjectCollidedLeft(m_collisionRect.x + m_collisionRect.w + (sonic->hitBox().w / 2), sonic->onGround());
            }
        }
    }

    if (collisionState & SolidCollisionStates::CollisionInY) {
        /**
         * here we take sonics rect and subtract the vertical speed from it to see if sonic was above the
         * top of the object before the collision.  We need to do this because if sonic is falling, then his
         * x motion might push him into a platform and if his bottom is inside the platform he will incorrectly
         * snap up to the platform, resulting in very weird movement!
         */
        int lastPosY = sonic->rect().bottom() - sonic->verticalSpeed();

        if (sonic->verticalSpeed() > 0) {
            if ((m_solidity & Solidity::LeftRightBottom) || ((m_solidity & Solidity::Top) && (m_collisionRect.y > lastPosY))) {
                setStandingOnObject(yDistance + 1,  tileMap, camera, sonic);
            }
        }
    }

    return false;
}

auto Nedrysoft::Solid::setStandingOnObject(int distance, TileMap *tileMap, Camera *camera, Sonic *sonic) -> void {
    N_UNUSED(tileMap)
    N_UNUSED(camera)

    sonic->move(0.0f, static_cast<float>(distance + 1));

    sonic->setStandingOnObject(this);

    m_collisionFlags = SolidCollisionFlags::Top;
}

auto Nedrysoft::Solid::render(Camera *camera, ObjectRenderPriority priority) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    if ( (priority == ObjectRenderPriority::None) ||
         (priority != ObjectRenderPriority::NonPriorityOnly) ) {
         return;
    }

    SDL_Rect dest;

    int width;
    int height;
    Vector origin;

    SDL_Texture *texture;

    texture = m_animation.texture(&width, &height, &origin);

    dest.x = static_cast<int>(round(m_x) - origin.x() - camera->position().x());
    dest.y = static_cast<int>(round(m_y) - origin.y() - camera->position().y());
    dest.w = width;
    dest.h = height;

    int flipFlags = SDL_FLIP_NONE;

    flipFlags |= (m_mirrored ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    flipFlags |= (m_flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);

    SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, static_cast<SDL_RendererFlip>(flipFlags));

#if defined(DEBUG_DRAW_COLLISION_BOX)
    auto collisionBox = m_collisionRect;

    collisionBox.x -= static_cast<int>(camera->position().x());
    collisionBox.y -= static_cast<int>(camera->position().y());

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0xFF, 0xFF, 0xFF);

    SDL_RenderDrawRect(gameRenderer->renderer(), &collisionBox);

    collisionBox = Nedrysoft::Sonic::getInstance()->hitBox();

    collisionBox.x -= static_cast<int>(camera->position().x());
    collisionBox.y -= static_cast<int>(camera->position().y());

    SDL_RenderDrawRect(gameRenderer->renderer(), &collisionBox);
#endif
}

auto Nedrysoft::Solid::checkForCollisionAction(float xDistance, float yDistance, TileMap *tileMap, Camera *camera,
                                               class Sonic *sonic) -> bool {
    N_UNUSED(yDistance)
    N_UNUSED(tileMap)
    N_UNUSED(camera)
    N_UNUSED(sonic)

    return false;
}

auto Nedrysoft::Solid::setCollisionEnabled(bool collisionEnabled) -> void {
    m_collisionEnabled = collisionEnabled;
}

auto Nedrysoft::Solid::isCollisionEnabled() const -> bool {
    return m_collisionEnabled;
}

auto Nedrysoft::Solid::objectUpdating(TileMap *tileMap, Camera *camera, Sonic *sonic) -> void {
    N_UNUSED(tileMap)
    N_UNUSED(camera)
    N_UNUSED(sonic)
}

auto Nedrysoft::Solid::createHitBox(AnimationPlayer *animationPlayer, int dx, int dy, int dw, int dh) -> void {
    int width;
    int height;

    Vector origin;

    animationPlayer->texture(&width, &height, &origin);

    m_collisionRect.x = static_cast<int>(round(m_x - origin.x())) + dx;
    m_collisionRect.w = width + dw;
    m_collisionRect.y = static_cast<int>(round(m_y - (static_cast<float>(height) / 2.0f))) + dy;
    m_collisionRect.h = height + dh;
}

auto Nedrysoft::Solid::hurtSonic(float yDistance, Sonic *sonic, bool overrideInvulnerability) -> void {
    sonic->hurt(yDistance, Vector(m_x, m_y), overrideInvulnerability);
}

auto Nedrysoft::Solid::renderCollision(Camera *camera) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    N_UNUSED(camera);

    auto rect = m_collisionRect;

    rect.x -= camera->position().x();
    rect.y -= camera->position().y();

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0xFF, 0x00, 0x80);

    SDL_RenderFillRect(gameRenderer->renderer(), &rect);
}

auto Nedrysoft::Solid::reset() -> void {
    Object::reset();
}  