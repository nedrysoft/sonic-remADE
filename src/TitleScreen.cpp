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

#include "TitleScreen.h"


#include "GameRenderer.h"
#include "Image.h"

#include <iostream>

auto constexpr StartPositionY = 0xDE;
auto constexpr EndPositionY = 0x96;

auto constexpr SonicOffsetX = 4;
auto constexpr SonicOffsetY = 32;

auto constexpr ClipRectHeightAdjustment = 8;
auto constexpr MoveSpeed = 8;

Nedrysoft::TitleScreen::TitleScreen() {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_foreground = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-screen/title-screen.png", "PNG");

    m_pushStartButton = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-screen/title-screen-psb.png", "PNG");

    m_animations = Animations::load(gameRenderer->renderer(), "./data/art/objects/title-screen/title-screen-sonic.json");

    reset();

    m_state = TitleScreenState::Idle;
}

auto Nedrysoft::TitleScreen::start() -> void {
    reset();

    m_state = TitleScreenState::FadeIn;
}

auto Nedrysoft::TitleScreen::end() -> void {
    m_state = TitleScreenState::FadeOut;
}

auto Nedrysoft::TitleScreen::reset() -> void {
    m_animations->start("title", &m_sonicAnimation);

    m_y = StartPositionY;

    m_frame = 0;

    m_alpha = 255;
}

auto Nedrysoft::TitleScreen::render() -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect dest;

    dest.x = (gameRenderer->viewportWidth() - m_foreground->width()) / 2;
    dest.y = (gameRenderer->viewportHeight() - m_foreground->height()) / 2;
    dest.w = m_foreground->width();
    dest.h = m_foreground->height();

    dest.x += 8;
    dest.y += 8;

    SDL_RenderCopyEx(gameRenderer->renderer(), m_foreground->texture(), nullptr, &dest, 0, nullptr, SDL_FLIP_NONE);

    if (m_state != TitleScreenState::FadeIn) {
        int width;
        int height;
        Vector origin;

        auto texture = m_sonicAnimation.texture(&width, &height, &origin);

        dest.w = width;
        dest.h = height;

        dest.x = (gameRenderer->viewportWidth() / 2) - (width / 2) + SonicOffsetX;
        dest.y = m_y - (height) - SonicOffsetY;

        SDL_Rect clipRect;

        clipRect.x = 0;
        clipRect.y = 0;
        clipRect.w = gameRenderer->viewportWidth();
        clipRect.h = (gameRenderer->viewportHeight() / 2) - ClipRectHeightAdjustment;

        SDL_RenderSetClipRect(gameRenderer->renderer(), &clipRect);
        SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, SDL_FLIP_NONE);
        SDL_RenderSetClipRect(gameRenderer->renderer(), nullptr);

        m_sonicAnimation.next();

        if (m_y > EndPositionY) {
            m_y -= MoveSpeed;
        }

        dest.x = (gameRenderer->viewportWidth() - m_pushStartButton->width()) / 2;
        dest.y = 180;
        dest.w = m_pushStartButton->width();
        dest.h = m_pushStartButton->height();

        if ((m_frame % 60) >= 30) {
            SDL_RenderCopyEx(gameRenderer->renderer(), m_pushStartButton->texture(), nullptr, &dest, 0, nullptr, SDL_FLIP_NONE);
        }

        m_frame++;
    }

    dest.x = 0;
    dest.y = 0;
    dest.w = gameRenderer->viewportWidth();
    dest.h = gameRenderer->viewportHeight();

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0x00, 0x00, 0x00, m_alpha);

    SDL_RenderFillRect(gameRenderer->renderer(), &dest);

    if (m_state == TitleScreenState::FadeIn) {
        m_alpha = std::max(0, m_alpha - 6);

        if (m_alpha == 0) {
            m_state = TitleScreenState::Normal;
        }
    } else if (m_state == TitleScreenState::FadeOut) {
        m_alpha = std::min(255, m_alpha + 6);

        if (m_alpha == 255) {
            m_state = TitleScreenState::Finished;
        }
    }
}

auto Nedrysoft::TitleScreen::finished() -> bool {
    return (m_state == TitleScreenState::Finished) || (m_state == TitleScreenState::Normal);
}