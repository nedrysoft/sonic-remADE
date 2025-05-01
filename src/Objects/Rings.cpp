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

#include "Rings.h"

#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Hud.h"
#include "Object.h"
#include "RingsFactory.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>

/**
 * @brief       Stores the X & Y spacing for a ring subtype
 */
struct RingSpacing {
    int x;                        //!<< Pixel offset in X
    int y;                        //!<< Pixel offset in Y
};

/**
 * @brief       The spacing between ring sub types
 *
 * @details     A single spring object may contain multiple rings, the high nybble specifies the
 *              spacing and the low nybble contains the number of rings.
 */

RingSpacing RingSpacings[] = {
    {.x =  16, .y = 0},             //!<< 0x0n         - horizontal tight
    {.x =  24, .y = 0},             //!<< 0x1n         - horizontal normal
    {.x =  32, .y = 0},             //!<< 0x2n         - horizontal wide
    {.x =  0,  .y = 16},            //!<< 0x3n         - vertical tight
    {.x =  0,  .y = 24},            //!<< 0x4n         - vertical normal
    {.x =  0,  .y = 32},            //!<< 0x5n         - vertical wide
    {.x =  16, .y = 16},            //!<< 0x6n         - diagonal
    {.x =  24, .y = 24},            //!<< 0x7n         - unused
    {.x =  32, .y = 32},            //!<< 0x8n         - diagonal tight
    {.x = -16, .y = 16},            //!<< 0x9n         - diagonal wide
    {.x = -24, .y = 24},            //!<< 0xAn         - unused
    {.x = -32, .y = 32},            //!<< 0xBn         - unused
    {.x =  16, .y = 8},             //!<< 0xCn         - unused
    {.x =  24, .y = 16},            //!<< 0xDn         - unused
    {.x = -16, .y = 8},             //!<< 0xEn         - unused
    {.x = -24, .y = 16}             //!<< 0xFn         - unused
};

auto constexpr HitboxWidthRadius = 6;
auto constexpr HitboxHeightRadius = 6;
auto constexpr SparklingFrames = 32;

Nedrysoft::AnimationPlayer Nedrysoft::Rings::m_ringAnimation = Nedrysoft::AnimationPlayer();

Nedrysoft::Rings::Rings(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Object("rings", subType, x, y, rememberState, mirrored, flipped) {

    reset();
}

auto Nedrysoft::Rings::update() -> void {
    m_ringAnimation.next();
}

auto Nedrysoft::Rings::initialise() -> void {
    Nedrysoft::RingsFactory::getInstance()->animations()->start("ring", &m_ringAnimation);
}

auto Nedrysoft::Rings::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    SDL_Rect sonicRect = sonic->hitBox();
    SDL_Rect objectRect;

    auto ringCount = ((m_subType & 0x0F) + 1);
    auto ringSpacing = ((m_subType & 0xF0) >> 4);

    objectRect.x = static_cast<int>(m_x) - HitboxWidthRadius;
    objectRect.y = static_cast<int>(m_y) - HitboxHeightRadius;
    objectRect.w = HitboxWidthRadius * 2;
    objectRect.h = HitboxHeightRadius * 2;

    SDL_Rect resultRect;

    for (auto ringIndex = 0; ringIndex < ringCount; ringIndex++) {
        switch(m_states[ringIndex].state) {
            case RingState::Uncollected: {
                if (sonic->isDying()) {
                    break;
                }

                if (SDL_IntersectRect(&objectRect, &sonicRect, &resultRect)) {
                    auto animations = Nedrysoft::RingsFactory::getInstance()->animations();

                    m_states[ringIndex].state = RingState::Sparkling;

                    animations->start("sparkle", &m_states[ringIndex].animation);

                    m_ringsRemaining--;

                    if (objectRect.x < static_cast<int>(sonic->position().x())) {
                        Nedrysoft::Audio::getInstance()->playSample(1, Nedrysoft::SoundId::RingLeft, true);
                    } else {
                        Nedrysoft::Audio::getInstance()->playSample(1, Nedrysoft::SoundId::RingRight, true);
                    }

                    sonic->addRings(1);
                }

                break;
            }

            case RingState::Sparkling: {
                if (m_states[ringIndex].timer--) {
                    m_states[ringIndex].animation.next();
                } else {
                    m_states[ringIndex].state = RingState::Collected;
                }

                break;
            }

            case RingState::Collected: {
                break;
            }
        }

        objectRect.x += RingSpacings[ringSpacing].x;
        objectRect.y += RingSpacings[ringSpacing].y;
    }

    return false;
}

auto Nedrysoft::Rings::render(Camera *camera, ObjectRenderPriority priority) -> void {
    SDL_Rect dest;
    int x = 0;
    int y = 0;

    if ( (priority == ObjectRenderPriority::None) ||
         (priority != ObjectRenderPriority::PriorityOnly) ) {
         return;
    }

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto ringCount = ((m_subType & 0x0F) + 1);
    auto ringSpacing = ((m_subType & 0xF0) >> 4);

    int ringIndex = 0;

    while(ringIndex < ringCount) {
        SDL_Texture *texture;
        Vector origin;

        switch(m_states[ringIndex].state) {
            case RingState::Sparkling: {
                texture = m_states[ringIndex].animation.texture(&dest.w, &dest.h, &origin);

                break;
            }

            case RingState::Uncollected: {
                texture = m_ringAnimation.texture(&dest.w, &dest.h, &origin);

                break;
            }

            default: {
                texture = nullptr;

                break;
            }
        }

        if (m_states[ringIndex].state != RingState::Collected) {
            assert(texture != nullptr);

            dest.x = static_cast<int>(static_cast<float>(x) + m_x - origin.x() - camera->position().x());
            dest.y = static_cast<int>(static_cast<float>(y) + m_y - origin.y() - camera->position().y());

            SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, SDL_FLIP_NONE);
        }

        x += RingSpacings[ringSpacing].x;
        y += RingSpacings[ringSpacing].y;

        ringIndex++;
    }
}

auto Nedrysoft::Rings::spawn(Camera *camera) -> bool {
    return m_ringsRemaining;
}

auto Nedrysoft::Rings::renderCollision(Camera *camera) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    N_UNUSED(camera);

    int x = 0;
    int y = 0;

    auto ringCount = ((m_subType & 0x0F) + 1);
    auto ringSpacing = ((m_subType & 0xF0) >> 4);

    for(auto ringIndex = 0; ringIndex < ringCount; ringIndex++) {
        if (m_states[ringIndex].state != RingState::Collected) {
            SDL_Rect rect;

            rect.x = static_cast<int>(static_cast<float>(x) + m_x - HitboxWidthRadius - camera->position().x());
            rect.y = static_cast<int>(static_cast<float>(y) + m_y - HitboxHeightRadius - camera->position().y());
            rect.w = HitboxWidthRadius * 2;
            rect.h = HitboxHeightRadius * 2;

            SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0xFF, 0x00, 0x80);

            SDL_RenderFillRect(gameRenderer->renderer(), &rect);

            x += RingSpacings[ringSpacing].x;
            y += RingSpacings[ringSpacing].y;
        }
    }
}

auto Nedrysoft::Rings::reset() -> void {
    Object::reset();

    m_ringsRemaining = ((m_subType & 0x0F) + 1);

    for (auto &ringState: m_states) {
        ringState.state = RingState::Uncollected;
        ringState.timer = SparklingFrames;
    }
}  

