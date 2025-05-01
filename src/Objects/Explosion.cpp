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

#include "Explosion.h"

#include "Animation.h"
#include "AnimationPlayer.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Hud.h"
#include "Object.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>

auto constexpr LifeSpanFrames = 7 * 5;

auto Nedrysoft::Explosion::update() -> void {
}

auto Nedrysoft::Explosion::initialise() -> void {

}

Nedrysoft::Explosion::Explosion(float x, float y) :
        Object("explosion", 0, x, y, false, false, false) {

    auto animations = Nedrysoft::Animations::load(Nedrysoft::GameRenderer::getInstance()->renderer(), "./data/art/objects/explosion/explosion.json");

    animations->start("explosion", &m_animation);

    m_lifespanTimer = LifeSpanFrames;
}

auto Nedrysoft::Explosion::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    m_animation.next();

    if (m_lifespanTimer) {
        m_lifespanTimer--;
    } else {
        return true;
    }

    return false;
}

auto Nedrysoft::Explosion::render(Camera *camera, ObjectRenderPriority priority) -> void {
    SDL_Rect dest;

    int x = 0;
    int y = 0;

    if (m_lifespanTimer <= 0) {
        return;
    }

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Texture *texture;
    Vector origin;

    texture = m_animation.texture(&dest.w, &dest.h, &origin);

    assert(texture != nullptr);

    dest.x = static_cast<int>(static_cast<float>(x) + m_x - origin.x() - camera->position().x());
    dest.y = static_cast<int>(static_cast<float>(y) + m_y - origin.y() - camera->position().y());

    SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, SDL_FLIP_NONE);
}

auto Nedrysoft::Explosion::spawn(Camera *camera) -> bool {
    return true;
}
