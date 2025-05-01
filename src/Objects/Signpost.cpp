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

#include "Signpost.h"

#include "SignpostFactory.h"

#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Object.h"
#include "Sonic.h"

#include <SDL2/SDL.h>

Nedrysoft::Signpost::Signpost(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Object("signpost", subType, x, y, rememberState, mirrored, flipped) {

    reset();
}

auto Nedrysoft::Signpost::update() -> void {

}

auto Nedrysoft::Signpost::initialise() -> void {

}

auto Nedrysoft::Signpost::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    m_animation.next();

    return false;
}

auto Nedrysoft::Signpost::render(Camera *camera, ObjectRenderPriority priority) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    if (priority != ObjectRenderPriority::NonPriorityOnly) {
         return;
    }

    SDL_Rect dest;
    Nedrysoft::Vector origin;

    auto texture = m_animation.texture(&dest.w, &dest.h, &origin);

    dest.x = static_cast<int>(round(m_x) - origin.x() - camera->position().x());
    dest.y = static_cast<int>(round(m_y) - origin.y() - camera->position().y());

    int flipFlags = SDL_FLIP_NONE;

    flipFlags |= (m_mirrored ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    flipFlags |= (m_flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);

    SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, static_cast<SDL_RendererFlip>(flipFlags));
}

auto Nedrysoft::Signpost::reset() -> void {
    Object::reset();

    Nedrysoft::SignpostFactory::getInstance()->animations()->start("spin1", &m_animation);
}  