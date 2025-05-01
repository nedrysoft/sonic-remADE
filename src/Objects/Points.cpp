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

#include "Points.h"

#include "Animation.h"
#include "AnimationPlayer.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Hud.h"
#include "Object.h"
#include "RingsFactory.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>

auto constexpr Gravity = 0.09375f;

Nedrysoft::Animations *Nedrysoft::Points::m_animations = nullptr;

Nedrysoft::Points::Points(int value, float x, float y) :
        Object("animal", 0, x, y, false, false, false),
        m_value(value) {

    reset();
}

auto Nedrysoft::Points::update() -> void {
}

auto Nedrysoft::Points::initialise() -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_animations = Animations::load(gameRenderer->renderer(), "./data/art/objects/points/points.json");
}

auto Nedrysoft::Points::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    SDL_Rect objectRect;
    SDL_Rect screenRect;
    SDL_Rect resultRect;

    m_animation.next();

    m_ySpeed += Gravity;

    if (m_ySpeed > 0) {
        return true;
    }

    m_y += m_ySpeed;

    screenRect.x = static_cast<int>(camera->position().x());
    screenRect.y = static_cast<int>(camera->position().y());
    screenRect.w = Nedrysoft::GameRenderer::getInstance()->viewportWidth();
    screenRect.h = Nedrysoft::GameRenderer::getInstance()->viewportHeight();

    objectRect.x = static_cast<int>(m_x);
    objectRect.y = static_cast<int>(m_y);
    objectRect.w = static_cast<int>(m_animation.width());
    objectRect.h = static_cast<int>(m_animation.height());

    if (!SDL_IntersectRect(&objectRect, &screenRect, &resultRect)) {
        return true;
    }

    return false;
}

auto Nedrysoft::Points::render(Camera *camera, ObjectRenderPriority priority) -> void {
    SDL_Rect dest;

    if (priority != ObjectRenderPriority::PriorityOnly) {
        return;
    }

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Texture *texture;
    Vector origin;

    texture = m_animation.texture(&dest.w, &dest.h, &origin);

    assert(texture != nullptr);

    dest.x = static_cast<int>(m_x - origin.x() - camera->position().x());
    dest.y = static_cast<int>(m_y - origin.y() - camera->position().y());

    SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, SDL_FLIP_NONE);
}

auto Nedrysoft::Points::spawn(Camera *camera) -> bool {
    return true;
}

auto Nedrysoft::Points::reset() -> void {
    Object::reset();

    m_ySpeed = -3.0f;

    m_animations->start(std::to_string(m_value), &m_animation);
}  
