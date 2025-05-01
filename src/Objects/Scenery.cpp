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

#include "Scenery.h"

#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Object.h"
#include "Sonic.h"

#include <SDL2/SDL.h>

std::vector<Nedrysoft::Image *> Nedrysoft::Scenery::m_images = std::vector<Nedrysoft::Image *>();

Nedrysoft::Scenery::Scenery(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Object("scenery", subType, x, y, rememberState, mirrored, flipped),
        m_sceneryType(SceneryType::BridgeStump) {

    reset();
}

auto Nedrysoft::Scenery::update() -> void {

}

auto Nedrysoft::Scenery::initialise() -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto image = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/ghz-bridge/ghz-bridge-stump.png", "PNG");

    m_images.push_back(image);
}


auto Nedrysoft::Scenery::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    return false;
}

auto Nedrysoft::Scenery::render(Camera *camera, ObjectRenderPriority priority) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    if ( (priority == ObjectRenderPriority::None) ||
         (priority != ObjectRenderPriority::PriorityOnly) ) {
         return;
    }

    SDL_Rect dest;

    auto image = m_images[static_cast<int>(m_sceneryType)];

    auto originX = static_cast<float>(image->width()) / 2.0f;
    auto originY = static_cast<float>(image->height()) / 2.0f;

    dest.x = static_cast<int>(round(m_x) - originX - camera->position().x());
    dest.y = static_cast<int>(round(m_y) - originY - camera->position().y());
    dest.w = image->width();
    dest.h = image->height();

    int flipFlags = SDL_FLIP_NONE;

    flipFlags |= (m_mirrored ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    flipFlags |= (m_flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);

    SDL_RenderCopyEx(gameRenderer->renderer(), m_images[0]->texture(), nullptr, &dest, 0, nullptr, static_cast<SDL_RendererFlip>(flipFlags));
}

auto Nedrysoft::Scenery::reset() -> void {
    Object::reset();
}  
