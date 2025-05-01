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

#include "ScatteredRing.h"

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

auto constexpr HitboxWidthRadius = 6;
auto constexpr HitboxHeightRadius = 6;
auto constexpr LifeSpanFrames = 256;
auto constexpr CollectionLockoutFrames = 64;
auto constexpr SparklingFrames = 32;
auto constexpr BounceVelocity = 0.75f;
auto constexpr Gravity = 0.09375f;

Nedrysoft::ScatteredRing::ScatteredRing(float x, float y, float xSpeed, float ySpeed) :
        Object("scattered-ring", 0, x, y, false, false, false),
        m_lifespanTimer(0),
        m_SparklingTimer(0),
        m_collectionTimer(0),
        m_xSpeed(xSpeed),
        m_ySpeed(ySpeed),
        m_state(RingState::None) {

    reset();
}

auto Nedrysoft::ScatteredRing::update() -> void {
}

auto Nedrysoft::ScatteredRing::initialise() -> void {

}

auto Nedrysoft::ScatteredRing::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    SDL_Rect sonicRect = sonic->hitBox();
    SDL_Rect objectRect;
    SDL_Rect screenRect;
    SDL_Rect resultRect;

    m_animation.next();

    m_x += m_xSpeed;
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

    objectRect.x = static_cast<int>(m_x);
    objectRect.y = static_cast<int>(m_y);
    objectRect.w = HitboxWidthRadius;
    objectRect.h = HitboxHeightRadius;

    float distance;

    switch(m_state) {
        case RingState::Bouncing: {
            m_ySpeed += Gravity;

            if ((m_lifespanTimer % 4) == 0) {
                if (m_ySpeed > 0) {
                    if (tileMap->findDown(Vector(m_x, m_y + (m_animation.height() / 2)), &distance)) {
                        if (distance <= 0) {
                            m_ySpeed *= -BounceVelocity;
                            m_y += distance;
                        }
                    }
                }
            }

            if (!m_collectionTimer) {
                if (SDL_IntersectRect(&objectRect, &sonicRect, &resultRect)) {
                    if (sonic->invulnerability() < 90) {
                        auto animations = Nedrysoft::RingsFactory::getInstance()->animations();

                        m_state = RingState::Sparkling;

                        animations->start("sparkle", &m_animation);

                        if (objectRect.x < static_cast<int>(sonic->position().x())) {
                            Nedrysoft::Audio::getInstance()->playSample(1, Nedrysoft::SoundId::RingLeft, true);
                        } else {
                            Nedrysoft::Audio::getInstance()->playSample(1, Nedrysoft::SoundId::RingRight, true);
                        }

                        sonic->addRings(1);

                        m_xSpeed = 0;
                        m_ySpeed = 0;
                    }
                }
            }

            break;
        }

        case RingState::Sparkling: {
            if (!m_SparklingTimer) {
                return true;
            }

            m_SparklingTimer--;

            break;
        }
    }

    if (m_collectionTimer) {
        m_collectionTimer--;
    }

    if (m_lifespanTimer) {
        m_lifespanTimer--;
    } else {
        return true;
    }

    return false;
}

auto Nedrysoft::ScatteredRing::render(Camera *camera, ObjectRenderPriority priority) -> void {
    SDL_Rect dest;

    int x = 0;
    int y = 0;

    if (m_SparklingTimer <= 0) {
        return;
    }

    if (m_lifespanTimer <= 0) {
        return;
    }

    if ( (m_state == RingState::Bouncing) && (priority != ObjectRenderPriority::NonPriorityOnly) ) {
        return;
    }

    if ( (m_state == RingState::Sparkling) && (priority != ObjectRenderPriority::PriorityOnly) ) {
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

auto Nedrysoft::ScatteredRing::spawn(Camera *camera) -> bool {
    return true;
}

auto Nedrysoft::ScatteredRing::reset() -> void {
    auto animations = Nedrysoft::RingsFactory::getInstance()->animations();

    m_lifespanTimer = LifeSpanFrames;
    m_SparklingTimer = SparklingFrames;
    m_collectionTimer = CollectionLockoutFrames;

    m_state = RingState::Bouncing;

    animations->start("ring", &m_animation, [&](const std::string, class Nedrysoft::AnimationPlayer *) {
        if (m_lifespanTimer) {
            return floor((static_cast<float>(LifeSpanFrames) * 2.0f) / (static_cast<float>(m_lifespanTimer)));
        }

        return 0.0f;
    });
}