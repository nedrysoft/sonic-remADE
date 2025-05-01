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

#include "MotoBug.h"

#include "Animal.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "Explosion.h"
#include "GameRenderer.h"
#include "MotoBugFactory.h"
#include "ObjectsManager.h"
#include "Points.h"
#include "Sonic.h"
#include "TileMap.h"
#include "Utils.h"

#include <iostream>

auto constexpr CollisionWidthRadius = 20;
auto constexpr CollisionHeightRadius = 16;

Nedrysoft::MotoBug::MotoBug(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Nedrysoft::Object("enemy-motobug", subType, x, y, rememberState, mirrored, flipped) {

    reset();
}

auto Nedrysoft::MotoBug::initialise() -> void {

}

auto Nedrysoft::MotoBug::update() -> void {

}

auto Nedrysoft::MotoBug::render(Camera *camera, ObjectRenderPriority priority) -> void {
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

auto Nedrysoft::MotoBug::update(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    int width;
    int height;
    Vector origin;

    m_animation.texture(&width, &height, &origin);

    float distance;

    m_x += m_xSpeed;

    if (m_x < (static_cast<float>(width) / 2.0f)) {
        m_x = static_cast<float>(width) / 2.0f;

        m_xSpeed = -m_xSpeed;
    }

    if (m_x >= static_cast<float>(tileMap->widthInPixels()) - (static_cast<float>(width) / 2.0f)) {
        m_x = static_cast<float>(tileMap->widthInPixels()) - (static_cast<float>(width) / 2.0f);

        m_xSpeed = -m_xSpeed;
    }

    if (tileMap->findDown(Vector(m_x, round(m_y + (static_cast<float>(height) / 2.0f))), &distance)) {
        if ((distance >= -8) && (distance < 12)) {
            m_y += distance;
        } else {
            m_xSpeed = -m_xSpeed;
        }
    } else {
        return true;
    }

    m_collisionRect.x = static_cast<int>(m_x) - CollisionWidthRadius;
    m_collisionRect.y = static_cast<int>(m_y) - CollisionHeightRadius;
    m_collisionRect.w = CollisionWidthRadius * 2;
    m_collisionRect.h = CollisionHeightRadius * 2;

    auto sonicRect = sonic->hitBox();

    if (m_alive) {
        if (SDL_HasIntersection(&sonicRect, &m_collisionRect)) {
            if (sonic->attacking()) {
                auto objectsManager = Nedrysoft::ObjectsManager::getInstance();

                objectsManager->addDynamicObject(new Animal(m_x, m_y));
                objectsManager->addDynamicObject(new Points(100, m_x, m_y));
                objectsManager->addDynamicObject(new Explosion(m_x, m_y));

                m_alive = false;

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

    m_animation.next();

    return false;
}

auto Nedrysoft::MotoBug::spawn(Camera *camera) -> bool {
    if (!m_alive) {
        return false;
    }

    auto animations = Nedrysoft::MotoBugFactory::getInstance()->animations();

    assert(animations);

    animations->start("walk", &m_animation);

    return true;
}

auto Nedrysoft::MotoBug::renderCollision(Camera *camera) -> void {
    if (!m_alive) {
        return;
    }
    
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect rect;

    rect.x = static_cast<int>(m_x) - CollisionWidthRadius - camera->position().x();
    rect.y = static_cast<int>(m_y) - CollisionHeightRadius - camera->position().y();
    rect.w = CollisionWidthRadius * 2;
    rect.h = CollisionHeightRadius * 2;

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0xFF, 0x00, 0x80);

    SDL_RenderFillRect(gameRenderer->renderer(), &rect);
}

auto Nedrysoft::MotoBug::reset() -> void {
    Object::reset();

    m_collisionRect = {.x = 0, .y = 0, .w = 0, .h = 0};
    m_animation = Nedrysoft::AnimationPlayer();
    m_xSpeed = -1.0f;
    m_alive = true;
}  
