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

#include "CrabMeat.h"

#include "Animations.h"
#include "Animal.h"
#include "Audio.h"
#include "Camera.h"
#include "CrabMeatFactory.h"
#include "CrabMeatProjectile.h"
#include "Explosion.h"
#include "GameRenderer.h"
#include "ObjectsManager.h"
#include "Points.h"
#include "Sonic.h"
#include "TileMap.h"
#include "Utils.h"

auto constexpr WalkMaximumFrames = 127;
auto constexpr WaitingFrames = 89;
auto constexpr FiringFrames = 59;

auto constexpr CollisionWidthRadius = 16;
auto constexpr CollisionHeightRadius = 16;
auto constexpr WidthRadius = 8;
auto constexpr HeightRadius = 16;

auto constexpr WalkingSpeed = 0.5f;

Nedrysoft::CrabMeat::CrabMeat(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Nedrysoft::Object("enemy-crabmeat", subType, x, y, rememberState, mirrored, flipped) {


    reset();
}

auto Nedrysoft::CrabMeat::initialise() -> void {

}

auto Nedrysoft::CrabMeat::update() -> void {

}

auto Nedrysoft::CrabMeat::render(Camera *camera, ObjectRenderPriority priority) -> void {
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

    flipFlags ^= (m_xSpeed > 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, static_cast<SDL_RendererFlip>(flipFlags));
}

auto Nedrysoft::CrabMeat::update(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    int width;
    int height;
    Vector origin;

    m_animation.texture(&width, &height, &origin);

    m_animation.next();

    float distance;

    switch(m_state) {
        case Walking: {
            if (m_timer) {
                m_x += m_xSpeed;

                m_timer--;
            } else {
                m_state = Waiting;
                m_timer = WaitingFrames;

                return checkCollision(tileMap, camera, sonic);
            }

            break;
        }

        case Waiting: {
            if (m_timer) {
                m_timer--;
            } else {
                /**
                 * if the crabmeat is not on screen then we don't bother firing, we just change direction
                 */
                if (!onScreen(camera)) {
                    turn();

                    break;
                }

                m_state = Firing;
                m_timer = FiringFrames;

                m_animations->start("firing", &m_animation);

                Nedrysoft::ObjectsManager::getInstance()->addDynamicObject(
                    new CrabMeatProjectile(m_x - (static_cast<float>(width) / 2.0f), m_y, -1)
                );

                Nedrysoft::ObjectsManager::getInstance()->addDynamicObject(
                    new CrabMeatProjectile(m_x + (static_cast<float>(width) / 2.0f), m_y, 1)
                );
            }

            break;
        }

        case Firing: {
            if (m_timer) {
                m_timer--;

                return checkCollision(tileMap, camera, sonic);
            }

            m_state = Walking;
            m_timer = WalkMaximumFrames;
            m_animations->start("walk", &m_animation);

            return checkCollision(tileMap, camera, sonic);
        }
    }

    if (m_x < (static_cast<float>(width) / 2.0f)) {
        m_x = static_cast<float>(width) / 2.0f;

        turn();

        return checkCollision(tileMap, camera, sonic);
    }

    if (m_x >= static_cast<float>(tileMap->widthInPixels()) - (static_cast<float>(width) / 2.0f)) {
        m_x = static_cast<float>(tileMap->widthInPixels()) - (static_cast<float>(width) / 2.0f);

        turn();

        return checkCollision(tileMap, camera, sonic);
    }

    if (m_xSpeed < 0) {
        if (tileMap->findDown(Vector(m_x - CollisionWidthRadius, round(m_y + (static_cast<float>(height) / 2.0f))), &distance)) {
            if ((distance >= -8) && (distance <= 12)) {
                m_y += distance;
            } else {
                turn();
            }
        } else {
            turn();
        }
    }

    if (m_xSpeed > 0) {
        if (tileMap->findDown(Vector(m_x + CollisionWidthRadius, round(m_y + (static_cast<float>(height) / 2.0f))), &distance)) {
            if ((distance >= -8) && (distance <= 12)) {
                m_y += distance;
            } else {
                turn();
            }
        } else {
            turn();
        }
    }

    return checkCollision(tileMap, camera, sonic);
}

auto Nedrysoft::CrabMeat::checkCollision(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    m_collisionRect.x = static_cast<int>(m_x) - CollisionWidthRadius;
    m_collisionRect.y = static_cast<int>(m_y) - CollisionHeightRadius;
    m_collisionRect.w = CollisionWidthRadius * 2;
    m_collisionRect.h = CollisionHeightRadius * 2;

    auto sonicRect = sonic->hitBox();

    if (m_alive) {
        if (SDL_HasIntersection(&sonicRect, &m_collisionRect)) {
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

auto Nedrysoft::CrabMeat::spawn(Camera *camera) -> bool {
    if (!m_alive) {
        return false;
    }

    m_animations = Nedrysoft::CrabMeatFactory::getInstance()->animations();

    assert(m_animations);

    m_animations->start("walk", &m_animation);

    return true;
}

auto Nedrysoft::CrabMeat::turn() -> void {
    m_state = Waiting;
    m_timer = WaitingFrames;
    m_xSpeed = -m_xSpeed;

    m_animations->start("stand", &m_animation);
}

auto Nedrysoft::CrabMeat::renderCollision(Camera *camera) -> void {
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

auto Nedrysoft::CrabMeat::onScreen(Camera *camera) -> bool {
    int width;
    int height;
    Vector origin;

    m_animation.texture(&width, &height, &origin);

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect objectRect;
    SDL_Rect screenRect;
    SDL_Rect intersectRect;

    objectRect.x = m_x - (static_cast<float>(width) / 2.0f);
    objectRect.y = m_y - (static_cast<float>(height) / 2.0f);
    objectRect.w = width;
    objectRect.h = height;

    screenRect.x = camera->position().x();
    screenRect.y = camera->position().y();
    screenRect.w = gameRenderer->viewportWidth();
    screenRect.h = gameRenderer->viewportHeight();

    if (SDL_IntersectRect(&objectRect, &screenRect, &intersectRect)) {
       return true;
   }

   return false;
}

auto Nedrysoft::CrabMeat::reset() -> void {
    Object::reset();

    m_collisionRect = {.x = 0, .y = 0, .w = 0, .h = 0};
    m_animation = Nedrysoft::AnimationPlayer();
    m_xSpeed = -WalkingSpeed;
    m_alive = true;
    m_timer = WalkMaximumFrames;
    m_state = Walking;
}  
