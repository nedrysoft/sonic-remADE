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

#include "Bridge.h"

#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Object.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>

auto constexpr HitboxWidthRadius = 6;
auto constexpr HitboxHeightRadius = 6;

Nedrysoft::Image * Nedrysoft::Bridge::m_logImage = nullptr;

auto Nedrysoft::Bridge::update() -> void {

}

auto Nedrysoft::Bridge::initialise() -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_logImage = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/ghz-bridge/ghz-bridge-log.png", "PNG");
}

Nedrysoft::Bridge::Bridge(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Object("bridge", subType, x, y, rememberState, mirrored, flipped),
        m_collisionRect {.x = 0, .y = 0, .w = 0, .h = 0} {

    m_logCount = m_subType;

    reset();
}

auto Nedrysoft::Bridge::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    N_UNUSED(tileMap)

    SDL_Rect sonicRect;
    SDL_Rect resultRect;

    if (m_logImage == nullptr) {
        return false;
    }

    auto startY = m_y - (static_cast<float>(m_logImage->height()) / 2.0f);

    sonicRect.x = static_cast<int>(sonic->rect().x());
    sonicRect.y = static_cast<int>(sonic->rect().y());
    sonicRect.w = static_cast<int>(sonic->rect().width());
    sonicRect.h = static_cast<int>(sonic->rect().height());

    if (!SDL_IntersectRect(&m_collisionRect, &sonicRect, &resultRect)) {
        if (sonic->standingOnObject() == this) {
            sonic->setStandingOnObject(nullptr);
        }

        for (auto &log : m_logPositions) {
            log = std::max(m_collisionRect.y, log - 1);
        }

        return false;
    } else {
        if (sonic->isJumpingUp()) {
            return false;
        } else {
            sonic->setStandingOnObject(this);
        }
    }

    if (sonic->isDying()) {
        return false;
    }

    int currentLog = static_cast<int>(((sonic->rect().centre().x() - static_cast<float>(m_startX)) / static_cast<float>(m_endX - m_startX)) * static_cast<float>(m_logCount));

    currentLog = std::max(0, std::min(currentLog, static_cast<int>(m_maxLogDips.size()-1)));

    auto maxDip = static_cast<float>(m_maxLogDips[currentLog]);

    m_logPositions[currentLog] = static_cast<int>(startY + maxDip);

    for (int i = 0; i < currentLog; i++) {
        auto sine = sin(degToRad(90 * static_cast<float>(i + 1) / static_cast<float>(currentLog)));

        m_logPositions[i] = static_cast<int>(startY + (maxDip * sine));
    }

    for (int i = m_logCount - 1; i > currentLog; i--) {
        auto sine = sin(degToRad(90 * static_cast<float>(m_logCount - i) / static_cast<float>(m_logCount - currentLog + 1)));

        m_logPositions[i] = static_cast<int>(startY + (maxDip * sine));
    }

    sonic->move(0.0f, static_cast<float>((m_logPositions[currentLog] - (sonicRect.y + sonicRect.h)) + 1));

    return false;
}

auto Nedrysoft::Bridge::render(Camera *camera, ObjectRenderPriority priority) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    if ( (priority == ObjectRenderPriority::None) ||
         (priority != ObjectRenderPriority::NonPriorityOnly) ) {
         return;
    }

    if (!m_logImage) {
        return;
    }

    SDL_Rect dest = {
        .x = m_startX,
        .y = m_startY,
        .w = m_logImage->width(),
        .h = m_logImage->height()
    };

    int flipFlags = SDL_FLIP_NONE;

    flipFlags |= (m_mirrored ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    flipFlags |= (m_flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);

    SDL_RenderCopyEx(gameRenderer->renderer(), m_logImage->texture(), nullptr, &dest, 0, nullptr, static_cast<SDL_RendererFlip>(flipFlags));

    for (int currentLog = 0; currentLog < m_logCount; currentLog++) {
        dest.x = m_startX + (currentLog * m_logImage->width()) - static_cast<int>(camera->position().x());
        dest.y = m_logPositions[currentLog] - static_cast<int>(camera->position().y());

        SDL_RenderCopyEx(gameRenderer->renderer(), m_logImage->texture(), nullptr, &dest, 0, nullptr, static_cast<SDL_RendererFlip>(flipFlags));
    }
}

auto Nedrysoft::Bridge::reset() -> void {
    Object::reset();

    auto originX = m_logImage ? m_logImage->width() / 2 : 0;
    auto originY = m_logImage ? m_logImage->height() / 2 : 0;
    auto width = m_logImage ? m_logImage->width() : 0;
    auto height = m_logImage ? m_logImage->height() : 0;

    m_startX = static_cast<int>(static_cast<int>(m_x) - ((m_logCount / 2) * width) - originX);
    m_endX = m_startX + (m_logCount * width);
    m_startY = static_cast<int>(static_cast<int>(m_y) - originY);

    m_maxLogDips.resize(m_logCount);
    m_logPositions.resize(m_logCount);

    for (int dipIndex = 0; dipIndex < m_logCount / 2; dipIndex++) {
        m_maxLogDips[dipIndex] = (dipIndex + 1) * 2;
        m_maxLogDips[m_logCount - dipIndex - 1] = (dipIndex + 1) * 2;
    }

    m_collisionRect.x = m_startX;
    m_collisionRect.w = m_endX - m_startX;
    m_collisionRect.y = m_startY;
    m_collisionRect.h = height;
}   
