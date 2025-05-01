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

#include "Camera.h"

#include "GameRenderer.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>

constexpr auto LeftBorder = 144;
constexpr auto RightBorder = 160;
constexpr auto DefaultVerticalFocalPoint = 96;
constexpr auto VerticalBorder = 32;
constexpr auto HorizontalSpeedCap = 16;

Nedrysoft::Camera::Camera() :
        m_x(0.0f),
        m_y(0.0f),
        m_worldWidth(0.0f),
        m_worldHeight(0.0f),
        m_verticalFocalPoint(DefaultVerticalFocalPoint) {

}

auto Nedrysoft::Camera::update(class Sonic *player) -> void {
    float screenX, screenY, newPositionX, newPositionY, verticalCap;

    /**
     * the Y limit changes depending on where in the level the player is, this routine checks whether the player
     * has passed into the next or previous segment.
     */

    if ((player->rect().centre().x() < m_currentLimit->startX) || ((player->rect().centre().x() > m_currentLimit->endX))) {
        if ((player->rect().centre().x() < m_currentLimit->startX)) {
            while (m_currentLimit != m_limits.begin()) {
                m_currentLimit--;

                if ((player->rect().centre().x() >= m_currentLimit->startX) && (player->rect().centre().x() < m_currentLimit->endX)) {
                    break;
                }
            }
        } else {
            while (m_currentLimit != m_limits.end()) {
                m_currentLimit++;

                if ((player->rect().centre().x() >= m_currentLimit->startX) && (player->rect().centre().x() < m_currentLimit->endX)) {
                    break;
                }
            }
        }
    }

    screenX = player->rect().centre().x() - m_x;
    screenY = player->rect().centre().y() - m_y - (player->rect().height() / 2.0f);

    if (screenX > RightBorder) {
        newPositionX = m_x + std::min<float>(HorizontalSpeedCap, screenX - RightBorder);
    } else if (screenX < LeftBorder) {
        newPositionX = m_x + std::max<float>(-HorizontalSpeedCap, screenX - LeftBorder);
    } else {
        newPositionX = m_x;
    }


    if (m_verticalFocalPoint == DefaultVerticalFocalPoint) {
        if (abs(player->groundSpeed()) < 8.0f) {
            verticalCap = 6.0f;
        } else {
            verticalCap = 16.0f;
        }
    } else  {
        verticalCap = 2.0f;
    }

    if (player->onGround()) {
        if (screenY > DefaultVerticalFocalPoint) {
            newPositionY = m_y + std::min(verticalCap, (screenY - DefaultVerticalFocalPoint));
        } else {
            newPositionY = m_y - std::min(verticalCap, (DefaultVerticalFocalPoint - screenY));
        }
    } else {
        int topBorder = DefaultVerticalFocalPoint - VerticalBorder;
        int bottomBorder = DefaultVerticalFocalPoint + VerticalBorder;

        if (screenY < static_cast<float>(topBorder)) {
            newPositionY = m_y - (static_cast<float>(topBorder) - screenY);
        } else if (screenY > static_cast<float>(bottomBorder)) {
            newPositionY = m_y + (screenY - static_cast<float>(bottomBorder));
        } else {
            newPositionY = m_y;
        }
    }

    /**
     * we check and adjust the camera position if the newly calculated position would result in the camera being
     * off the bounds of the world.
     */

    clipPosition(&newPositionX, &newPositionY);

    m_x = newPositionX;
    m_y = newPositionY;
}

auto Nedrysoft::Camera::setWorldSize(int width, int height) -> void {
    m_worldWidth = static_cast<float>(width);
    m_worldHeight = static_cast<float>(height);
}

auto Nedrysoft::Camera::renderDebugOverlay() const -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect rect;

    rect.x = static_cast<int>(gameRenderer->viewportWidth() / 2) - 10;
    rect.y = static_cast<int>(m_verticalFocalPoint) - 32;
    rect.w = 10 * 2;
    rect.h = 64;

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0xFF, 0xFF, 0xFF);

    SDL_RenderDrawRect(gameRenderer->renderer(), &rect);

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0x00, 0xFF, 0x00, 0xFF);

    SDL_RenderDrawLine(gameRenderer->renderer(), rect.x, static_cast<int>(m_verticalFocalPoint), rect.x + rect.w, static_cast<int>(m_verticalFocalPoint));
}

auto Nedrysoft::Camera::position() const -> Vector {
    return {m_x, m_y};
}

auto Nedrysoft::Camera::setLimits(std::list<Limit> limits) -> void {
    m_limits = std::move(limits);

    m_currentLimit = m_limits.begin();
}

auto Nedrysoft::Camera::setInitialPosition(const Vector &position) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto x = position.x() - (static_cast<float>(gameRenderer->viewportWidth()) / 2.0f);
    auto y = position.y() - m_verticalFocalPoint;

    /*
     * we try to set the initial position to be centered horizontally and positioned on the focus point vertically,
     * the actual final position is clipped horizontally to the map limits.  Vertically the map is artificially limited
     * to the passed in maximum Y.
     */

    clipPosition(&x, &y);

    m_x = x;
    m_y = y;
}

auto Nedrysoft::Camera::clipPosition(float *positionX, float *positionY) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    if (*positionX < 0.0f) {
        *positionX = 0.0f;
    } else if (*positionX > (m_worldWidth - static_cast<float>(gameRenderer->viewportWidth()))) {
        *positionX = m_worldWidth - static_cast<float>(gameRenderer->viewportWidth());
    }

    if (*positionY < 0.0f) {
        *positionY = 0.0f;
    } else if (*positionY > m_currentLimit->y - static_cast<float>(gameRenderer->viewportHeight())) {
        *positionY =  m_currentLimit->y - static_cast<float>(gameRenderer->viewportHeight());
    }
}