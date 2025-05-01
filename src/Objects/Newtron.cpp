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

#include "Newtron.h"

#include "Animations.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "NewtronFactory.h"
#include "TileMap.h"

auto constexpr ObjectXSpeed = 4.0f;
auto constexpr TurnFrames = 60;

Nedrysoft::Newtron::Newtron(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Nedrysoft::Object("enemy-newtron", subType, x, y, rememberState, mirrored, flipped) {

}

auto Nedrysoft::Newtron::initialise() -> void {

}

auto Nedrysoft::Newtron::update() -> void {

}

auto Nedrysoft::Newtron::render(Camera *camera, ObjectRenderPriority priority) -> void {
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

    // this kludge is here because there's a frame that is sized different, so we need to offset that frame so that it appears in the right
    // place.

    int off;

    if (m_xSpeed > 0) {
        off = 56 - width;
    } else {
        off = 0;
    }

    dest.x = static_cast<int>(round(m_x) - origin.x() - camera->position().x() + off);
    dest.y = static_cast<int>(round(m_y) - origin.y() - camera->position().y());
    dest.w = width;
    dest.h = height;

    int flipFlags = SDL_FLIP_NONE;

    flipFlags |= (m_mirrored ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    flipFlags |= (m_flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);

    if (!m_mirrored) {
        flipFlags ^= (m_xSpeed > 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    } else {
        flipFlags ^= (m_xSpeed > 0) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    }

    SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, static_cast<SDL_RendererFlip>(flipFlags));
}

auto Nedrysoft::Newtron::update(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool {
    if (m_turnTimer) {
        m_turnTimer--;

        return false;
    }

    int width;
    int height;
    Vector origin;

    int maxDistance = (Nedrysoft::GameRenderer::getInstance()->viewportWidth() * 2.0f);

    m_animation.texture(&width, &height, &origin);

    m_x += m_xSpeed;

    if (m_x < (m_xPosition - maxDistance)) {
        m_x = m_xPosition - maxDistance;

        m_xSpeed = -m_xSpeed;

        m_turnTimer = TurnFrames;

        return false;
    }

    if (m_x > m_xPosition) {
        m_x = m_xPosition;

        m_xSpeed = -m_xSpeed;

        m_turnTimer = TurnFrames;

        return false;
    }

    m_animation.next();

    return false;
}

auto Nedrysoft::Newtron::spawn(Camera *camera) -> bool {
    if (!m_alive) {
        return false;
    }

    m_x = m_xPosition;
    m_xSpeed = -ObjectXSpeed;

    auto animations = Nedrysoft::NewtronFactory::getInstance()->animations();

    assert(animations);

    animations->start("fly1", &m_animation);

    return true;
}

auto Nedrysoft::Newtron::reset() -> void {
    Object::reset();

    m_collisionRect = {.x = 0, .y = 0, .w = 0, .h = 0};
    m_animation = Nedrysoft::AnimationPlayer();
    m_xSpeed = -ObjectXSpeed;
    m_xPosition = m_x;
    m_alive = true;
    m_turnTimer = 0;
}  
