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

#include "BuzzBomber.h"

#include "Animations.h"
#include "Animal.h"
#include "Audio.h"
#include "BuzzBomberFactory.h"
#include "BuzzBomberProjectile.h"
#include "Camera.h"
#include "Explosion.h"
#include "GameRenderer.h"
#include "ObjectsManager.h"
#include "Points.h"
#include "Sonic.h"
#include "TileMap.h"
#include "Utils.h"

auto constexpr ObjectXSpeed = 4.0f;
auto constexpr ProjectileXSpeed = 2;
auto constexpr ProjectileYSpeed = 2;
auto constexpr ProjectileYOffset = 8;

auto constexpr CollisionWidthRadius = 24.0f;
auto constexpr CollisionHeightRadius = 12.0f;

auto constexpr FiringDistance = 96;

auto constexpr TurnFrames = 60;
auto constexpr PreFiringFrames = 30;
auto constexpr PostFiringFrames = 30
;
Nedrysoft::BuzzBomber::BuzzBomber(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Nedrysoft::Object("enemy-buzzbomber", subType, x, y, rememberState, mirrored, flipped) {

    reset();
}

auto Nedrysoft::BuzzBomber::initialise() -> void {

}

auto Nedrysoft::BuzzBomber::update() -> void {

}

auto Nedrysoft::BuzzBomber::render(Camera *camera, ObjectRenderPriority priority) -> void {
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

    int posX =  static_cast<int>(round(m_x) - origin.x() - camera->position().x());
    int posY =  static_cast<int>(round(m_y) - origin.y() - camera->position().y());

    dest.x = posX;
    dest.y = posY;

    dest.w = width;
    dest.h = height;

    int flipFlags = SDL_FLIP_NONE;

    flipFlags |= (m_mirrored ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    flipFlags |= (m_flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);

    flipFlags ^= (m_xSpeed > 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (flipFlags & SDL_FLIP_HORIZONTAL) {
        dest.x = static_cast<int>(round(m_x) - (static_cast<float>(dest.w) - origin.x()) - camera->position().x());
    }

    SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, static_cast<SDL_RendererFlip>(flipFlags));
}

auto Nedrysoft::BuzzBomber::update(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    switch(m_state) {
        case Flying: {
            return updateFlying(tileMap, camera, sonic);
        }

        case Firing: {
            return updateFiring(tileMap, camera, sonic);
        }

        case PreparingToFire: {
            return updatePreparingToFire(tileMap, camera, sonic);
        }

        case PreparingToMove: {
            return updatePreparingToMove(tileMap, camera, sonic);
        }

        case Turning: {
            return updateTurning(tileMap, camera, sonic);
        }

        default: {
            break;
        }
    }

    assert(false);

    return false;
}

auto Nedrysoft::BuzzBomber::updateFlying(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    N_UNUSED(tileMap)

    int width;
    int height;

    m_animation.next();

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto screenLeft = camera->position().x();
    auto screenRight = camera->position().x() + static_cast<float>(gameRenderer->viewportWidth());
    auto screenBottom = camera->position().y() + static_cast<float>(gameRenderer->viewportHeight());

    int maxDistance = (gameRenderer->viewportWidth() * 2);

    m_animation.texture(&width, &height);

    m_x += m_xSpeed;

    /**
     * start firing...
     *
     * we only start if:
     *
     *   1. We're not locked out from firing.
     *   2. If the x delta between the player and the bomber is less than "FiringDistance"
     *   3. If the bomber is on screen.
     *
     * The firing lock is set when a missile is fired, the lock is only removed when:
     *
     *   1. It hit sonic and he's dead.
     *   2. The bomber is de-spawned.
     *   3. The missile has gone off screen.
     *   4. The bomber changed direction.
     */

    if ((!m_firingLock) && ((fabs(sonic->position().x()) - m_x) < FiringDistance)) {
        if ( (m_x > screenLeft + m_animation.width()) &&
             (m_x < screenRight - m_animation.width()) &&
             (m_y < screenBottom + m_animation.height()) &&
             (m_y < sonic->position().y()) ) {
                return setState(PreparingToFire, sonic);
        }
    }

    /**
     * check if the bomber has gone off the right of the screen.
     */

    if (m_x < (m_xPosition - static_cast<float>(maxDistance))) {
        m_x = m_xPosition - static_cast<float>(maxDistance);

        m_firingLock = false;

        return setState(Turning, sonic);
    }

    /**
     * check if the bomber has gone off the left of the screen.
     */

    if (m_x > static_cast<float>(m_xPosition)) {
        m_x = m_xPosition;

        m_firingLock = false;

        return setState(Turning, sonic);
    }

    return checkCollision(sonic);
}

auto Nedrysoft::BuzzBomber::updateFiring(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    N_UNUSED(tileMap)
    N_UNUSED(camera)
    N_UNUSED(sonic)

    int width;
    int height;
    Vector origin;

    auto animations = Nedrysoft::BuzzBomberFactory::getInstance()->animations();

    assert(animations != nullptr);

    m_animation.texture(&width, &height, &origin);

    int projectileWidth, projectileHeight;

    Vector projectileOrigin;

    AnimationPlayer projectileAnimation;

    animations->start("flare", &projectileAnimation);

    projectileAnimation.texture(&projectileWidth, &projectileHeight, &projectileOrigin);

    float projectileX;
    float projectileY;
    float projectileXSpeed;

    if (m_xSpeed > 0.0f) {
        projectileX = m_x + origin.x() + projectileOrigin.x() - static_cast<float>(projectileWidth);

        projectileXSpeed = ProjectileXSpeed;
    } else {
        projectileX = m_x - (origin.x() + projectileOrigin.x());

        projectileXSpeed = -ProjectileXSpeed;
    }

    projectileY = m_y + static_cast<float>(projectileHeight + ProjectileYOffset);

    Nedrysoft::ObjectsManager::getInstance()->addDynamicObject(
        new BuzzBomberProjectile(projectileX, projectileY, projectileXSpeed, ProjectileYSpeed)
    );

    m_animation.next();

    return setState(PreparingToMove, sonic);
}

auto Nedrysoft::BuzzBomber::updateTurning(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    N_UNUSED(tileMap)
    N_UNUSED(camera)
    N_UNUSED(sonic)

    m_animation.next();

    if (m_timer) {
        m_timer--;

        return checkCollision(sonic);
    }

    return setState(Flying, sonic);
}

auto Nedrysoft::BuzzBomber::updatePreparingToFire(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    N_UNUSED(tileMap)
    N_UNUSED(camera)
    N_UNUSED(sonic)

    m_animation.next();

    if (!m_timer--) {
        return setState(Firing, sonic);
    }

    return checkCollision(sonic);
}

auto Nedrysoft::BuzzBomber::updatePreparingToMove(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    N_UNUSED(tileMap)
    N_UNUSED(camera)
    N_UNUSED(sonic)

    m_animation.next();

    if (!m_timer--) {
        return setState(Flying, sonic);
    }

    return checkCollision(sonic);
}

auto Nedrysoft::BuzzBomber::setState(int state, class Sonic *sonic) -> int {
    auto animations = Nedrysoft::BuzzBomberFactory::getInstance()->animations();

    assert(animations);

    m_state = state;

    switch(m_state) {
        case Turning: {
            m_xSpeed = -m_xSpeed;

            m_timer = TurnFrames;

            animations->start("fly1", &m_animation);

            return checkCollision(sonic);
        }

        case PreparingToFire: {
            m_timer = PreFiringFrames;

            animations->start("fires", &m_animation);

            return checkCollision(sonic);
        }

        case PreparingToMove: {
            m_timer = PostFiringFrames;

            return checkCollision(sonic);
        }

        case Firing: {
            m_firingLock = true;

            return checkCollision(sonic);
        }

        case Flying: {
            animations->start("fly2", &m_animation);

            return checkCollision(sonic);
        }

        default: {
            break;
        }
    }

    return checkCollision(sonic);
}

auto Nedrysoft::BuzzBomber::spawn(Camera *camera) -> bool {
    if (!m_alive) {
        return false;
    }

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto screenLeft = camera->position().x();
    auto screenRight = camera->position().x() + static_cast<float>(gameRenderer->viewportWidth());
    auto screenTop = camera->position().y();
    auto screenBottom = camera->position().y() + static_cast<float>(gameRenderer->viewportHeight());

    /**
     * don't spawn if the spawn location is on screen otherwise the buzz bomber will just appear from
     * nowhere.
     */

    if ( (m_xPosition >= screenLeft) &&
         (m_xPosition <= screenRight) &&
         (m_y >= screenTop) &&
         (m_y <= screenBottom) ) {

        return false;
    }

    m_firingLock = false;
    m_timer = 0;
    m_x = m_xPosition;
    m_xSpeed = -ObjectXSpeed;

    setState(Flying);

    auto animations = Nedrysoft::BuzzBomberFactory::getInstance()->animations();

    assert(animations);

    return true;
}

auto Nedrysoft::BuzzBomber::checkCollision(class Sonic *sonic) -> bool {
    SDL_Rect rect;

    if (!sonic) {
        return false;
    }

    rect.x = static_cast<int>(m_x) - CollisionWidthRadius;
    rect.y = static_cast<int>(m_y) - CollisionHeightRadius;
    rect.w = CollisionWidthRadius * 2.0f;
    rect.h = CollisionHeightRadius * 2.0f;

    auto sonicRect = sonic->hitBox();

    if (m_alive) {
        if (SDL_HasIntersection(&sonicRect, &rect)) {
            if (sonic->attacking()) {
                auto objectsManager = Nedrysoft::ObjectsManager::getInstance();

                m_alive = false;

                objectsManager->addDynamicObject(new Animal(m_x, m_y));
                objectsManager->addDynamicObject(new Points(100, m_x, m_y));
                objectsManager->addDynamicObject(new Explosion(m_x, m_y));

                sonic->rebound(Sonic::ReboundMode::Badnik, Vector(m_x, m_y));

                Nedrysoft::Audio::getInstance()->playSample(1, Nedrysoft::SoundId::Break);

                return true;
            } else {
                sonic->hurt(0, (Vector(m_x, m_y)));

                return false;
            }
        }
    } else {
        return true;
    }

    return false;
}

auto Nedrysoft::BuzzBomber::renderCollision(Camera *camera) -> void {
    if (!m_alive) {
        return;
    }
    
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    N_UNUSED(camera);

    SDL_Rect rect;

    rect.x = static_cast<int>(m_x) - CollisionWidthRadius - camera->position().x();
    rect.y = static_cast<int>(m_y) - CollisionHeightRadius - camera->position().y();
    rect.w = CollisionWidthRadius * 2;
    rect.h = CollisionHeightRadius * 2;

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0xFF, 0x00, 0x80);

    SDL_RenderFillRect(gameRenderer->renderer(), &rect);
}

auto Nedrysoft::BuzzBomber::reset() -> void {
    Object::reset();

    m_animation = Nedrysoft::AnimationPlayer();
    m_xSpeed = -ObjectXSpeed;
    m_xPosition = m_x;
    m_alive = true;
    m_timer = 0;
    m_state = BuzzBomberState::Flying;
    m_firingLock = false;
}   
