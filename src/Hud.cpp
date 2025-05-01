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

#include "Hud.h"

#include "GameRenderer.h"
#include "Sonic.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"

Nedrysoft::Hud::Hud() :
        m_framesElapsed(0),
        m_numbersImage(nullptr),
        m_scoreImage(nullptr),
        m_livesCounterNumbersImage(nullptr),
        m_ringsImage(nullptr),
        m_sonicLivesImage(nullptr),
        m_timeImage(nullptr) {

    initialise();
}

auto Nedrysoft::Hud::getInstance() -> Hud * {
    static Hud instance;

    return &instance;
}

auto Nedrysoft::Hud::reset() -> void {
    m_framesElapsed = 0;
}

auto Nedrysoft::Hud::initialise() -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_numbersImage = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/hud/numbers.png", "PNG");
    m_livesCounterNumbersImage = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/hud/lives-counter-numbers.png", "PNG");
    m_ringsImage = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/hud/rings.png", "PNG");
    m_scoreImage = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/hud/score.png", "PNG");
    m_sonicLivesImage = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/hud/sonic-lives.png", "PNG");
    m_timeImage = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/hud/time.png", "PNG");

    reset();
}

auto Nedrysoft::Hud::render(Sonic *sonic) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect dest;

    drawSprite(m_scoreImage, 16, 10, HudColour::Yellow);
    drawSprite(m_timeImage, 16, 26, HudColour::Yellow);

    if (sonic->rings() == 0) {
        if ((m_framesElapsed % 20) < 10) {
            drawSprite(m_ringsImage, 16, 42, HudColour::Yellow);
        } else {
            drawSprite(m_ringsImage, 16, 42, HudColour::Red);
        }
    } else {
        drawSprite(m_ringsImage, 16, 42, HudColour::Yellow);
    }

    drawNumber(sonic->points(), 7, m_scoreImage->width(), 10,HudColour::White);
    drawTime(m_framesElapsed / 60, m_scoreImage->width(), 26,HudColour::White);
    drawNumber(sonic->rings(), 4, m_scoreImage->width(), 42,HudColour::White);

    dest.x = 16;
    dest.y = gameRenderer->viewportHeight() - 24;
    dest.w = m_sonicLivesImage->width();
    dest.h = m_sonicLivesImage->height();

    SDL_RenderCopyEx(gameRenderer->renderer(), m_sonicLivesImage->texture(), nullptr, &dest, 0, nullptr, SDL_FLIP_NONE);

    drawLivesNumber(sonic->lives(), 4, 16, gameRenderer->viewportHeight() - 16,HudColour::White);

    m_framesElapsed++;
}

auto Nedrysoft::Hud::drawSprite(Image *image, int x, int y, HudColour colour) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect source;
    SDL_Rect dest;

    source.x = 0;
    source.y = static_cast<int>(colour) * (image->height() / 3);
    source.w = image->width();
    source.h = image->height() / 3;

    dest.x = x;
    dest.y = y;
    dest.w = source.w;
    dest.h = source.h;

    SDL_RenderCopyEx(gameRenderer->renderer(), image->texture(), &source, &dest, 0, nullptr, SDL_FLIP_NONE);
}

auto Nedrysoft::Hud::drawNumber(int value, int digits, int x, int y, HudColour colour) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    constexpr auto NumberWidth = 8;

    SDL_Rect source;
    SDL_Rect dest;

    source.x = 0;
    source.y = static_cast<int>(colour) * (m_numbersImage->height() / 3);
    source.w = NumberWidth;
    source.h = m_numbersImage->height() / 3;

    dest.x = x;
    dest.y = y;
    dest.w = source.w;
    dest.h = source.h;

    auto string = std::to_string(value);

    auto charIndex = static_cast<int>(string.length() - 1);

    for (int offset = digits; offset > 0; offset--) {
        if (charIndex < 0) {
            break;
        }

        source.x = (string[charIndex--] - '0') * NumberWidth;
        dest.x = x + ((offset + 1) * NumberWidth);

        SDL_RenderCopyEx(gameRenderer->renderer(), m_numbersImage->texture(), &source, &dest, 0, nullptr, SDL_FLIP_NONE);
    }
}

auto Nedrysoft::Hud::drawTime(int seconds, int x, int y, HudColour colour) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    constexpr auto NumberWidth = 8;

    SDL_Rect source;
    SDL_Rect dest;

    auto minutes = seconds / 60;
    seconds = seconds % 60;

    source.x = 0;
    source.y = static_cast<int>(colour) * (m_numbersImage->height() / 3);
    source.w = NumberWidth;
    source.h = m_numbersImage->height() / 3;

    dest.x = x;
    dest.y = y;
    dest.w = source.w;
    dest.h = source.h;

    auto string = std::to_string(minutes) + ":" + std::to_string(seconds / 10) +  std::to_string(seconds % 10);

    for (int offset = static_cast<int>(string.length() - 1); offset >= 0; offset--) {
        if (string[offset] == ':') {
            source.x = 80;
        } else {
            source.x = (string[offset] - '0') * NumberWidth;
        }

        dest.x = x + ((offset + 2) * NumberWidth);

        SDL_RenderCopyEx(gameRenderer->renderer(), m_numbersImage->texture(), &source, &dest, 0, nullptr, SDL_FLIP_NONE);
    }
}

auto Nedrysoft::Hud::drawLivesNumber(int value, int digits, int x, int y, HudColour colour) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    constexpr auto NumberWidth = 8;

    SDL_Rect source;
    SDL_Rect dest;

    source.x = 0;
    source.y = static_cast<int>(colour) * (m_livesCounterNumbersImage->height() / 3);
    source.w = NumberWidth;
    source.h = m_livesCounterNumbersImage->height() / 3;

    dest.x = x;
    dest.y = y;
    dest.w = source.w;
    dest.h = source.h;

    auto string = std::to_string(value);

    auto charIndex = static_cast<int>(string.length() - 1);

    for (int offset = digits; offset > 0; offset--) {
        if (charIndex < 0) {
            break;
        }

        source.x = (string[charIndex--] - '0') * NumberWidth;
        dest.x = x + ((offset + 1) * NumberWidth);

        SDL_RenderCopyEx(gameRenderer->renderer(), m_livesCounterNumbersImage->texture(), &source, &dest, 0, nullptr, SDL_FLIP_NONE);
    }
}

#pragma clang diagnostic pop