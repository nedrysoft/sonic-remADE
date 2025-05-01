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

#include "CrabMeatProjectile.h"

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

auto constexpr Gravity = 0.21875f;

auto Nedrysoft::CrabMeatProjectile::update() -> void {
}

auto Nedrysoft::CrabMeatProjectile::initialise() -> void {

}

Nedrysoft::CrabMeatProjectile::CrabMeatProjectile(float x, float y, float xSign) :
        Object("enemy-crabmeat-projectile", 0, x, y, false, false, false) {

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto animations = Animations::load(gameRenderer->renderer(), "./data/art/objects/enemy-crabmeat/enemy-crabmeat.json");

    animations->start("ball", &m_animation);

    if (xSign < 0) {
        m_x += m_animation.width() / 2.0f;
    } else {
        m_x -= m_animation.width() / 2.0f;
    }

    m_y -= m_animation.height() / 2.0f;

    m_xSpeed = 1.0f * xSign;
    m_ySpeed = -4.0f;
}

auto Nedrysoft::CrabMeatProjectile::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    int width, height;
    int widthRadius, heightRadius;

    m_animation.texture(&width, &height);

    widthRadius = width / 2;
    heightRadius = height / 2;

    m_animation.next();

    m_ySpeed += Gravity;

    m_x += m_xSpeed;
    m_y += m_ySpeed;

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto screenLeft = camera->position().x();
    auto screenRight = camera->position().x() + static_cast<float>(gameRenderer->viewportWidth());
    auto screenTop = camera->position().y();
    auto screenBottom = camera->position().y() + static_cast<float>(gameRenderer->viewportHeight());

    if ( (m_x < (screenLeft + widthRadius)) ||
         (m_x > (screenRight - widthRadius)) ||
         (m_y < (screenTop - heightRadius)) ||
         (m_y > (screenBottom + heightRadius)) ) {

        return true;
    }

    return false;
}

auto Nedrysoft::CrabMeatProjectile::render(Camera *camera, ObjectRenderPriority priority) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect dest;

    if (!m_animation.isNull()) {
        int projectileWidth, projectileHeight;

        auto projectileTexture = m_animation.texture(&projectileWidth, &projectileHeight);

        dest.x = static_cast<int>(m_x - camera->position().x() - (projectileWidth / 2.0f));
        dest.y = static_cast<int>(m_y - camera->position().y() - (projectileHeight / 2.0f));
        dest.w = projectileWidth;
        dest.h = projectileHeight;

        SDL_RenderCopyEx(gameRenderer->renderer(), projectileTexture, nullptr, &dest, 0, nullptr, SDL_FLIP_NONE);
    }
}

auto Nedrysoft::CrabMeatProjectile::spawn(Camera *camera) -> bool {
    return true;
}
