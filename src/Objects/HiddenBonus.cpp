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

#include "HiddenBonus.h"

#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Object.h"
#include "Sonic.h"

#include <SDL2/SDL.h>

auto constexpr TwirlSpeed = 0x30;

std::vector<Nedrysoft::Image *> Nedrysoft::HiddenBonus::m_images = std::vector<Nedrysoft::Image *>();

Nedrysoft::HiddenBonus::HiddenBonus(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Object("hidden-bonus", subType, x, y, rememberState, mirrored, flipped) {

    switch(subType) {
        case 1: {
            m_bonus = Bonus100;
            break;
        }

        case 2: {
            m_bonus = Bonus1000;
            break;
        }

        case 3: {
            m_bonus = Bonus10000;
            break;
        }
    }

    reset();
}

auto Nedrysoft::HiddenBonus::update() -> void {

}

auto Nedrysoft::HiddenBonus::initialise() -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_images.resize(3);

    m_images[Bonus100] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/hidden-bonus/hidden-bonus-100.png", "PNG");
    m_images[Bonus1000] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/hidden-bonus/hidden-bonus-1000.png", "PNG");
    m_images[Bonus10000] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/hidden-bonus/hidden-bonus-10000.png", "PNG");
}

auto Nedrysoft::HiddenBonus::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    return false;
}

auto Nedrysoft::HiddenBonus::render(Camera *camera, ObjectRenderPriority priority) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    if (priority != ObjectRenderPriority::PriorityOnly) {
         return;
    }

    SDL_Rect dest;

    auto image = m_images[static_cast<int>(m_bonus)];

    auto originX = 16.0f;
    auto originY = 12.0f;

    dest.x = static_cast<int>(round(m_x) - originX - camera->position().x());
    dest.y = static_cast<int>(round(m_y) - originY - camera->position().y());
    dest.w = image->width();
    dest.h = image->height();

    int flipFlags = SDL_FLIP_NONE;

    flipFlags |= (m_mirrored ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    flipFlags |= (m_flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);

    SDL_RenderCopyEx(gameRenderer->renderer(), m_images[0]->texture(), nullptr, &dest, 0, nullptr, static_cast<SDL_RendererFlip>(flipFlags));
}

auto Nedrysoft::HiddenBonus::reset() -> void {
    Object::reset();
}  
