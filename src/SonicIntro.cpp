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

#include "SonicIntro.h"

#include "FS.h"
#include "GameRenderer.h"

#include <Image.h>
#include <Magick++.h>
#include <SDL_image.h>
#if !defined(TARGET_WINDOWS)
#include <unistd.h>
#endif
#include <cstdint>
#include <iostream>

auto constexpr FadeFrames = 10;

auto handleEvents() -> bool;

auto Nedrysoft::SonicIntro::execute(SDL_Window *window, SDL_Renderer *renderer, int displayWidth, int displayHeight) -> void {
    auto image = Nedrysoft::Image(renderer, FS::toNative("./data/art/objects/ending-credits/ending-credits-sonicteam-andade.png"), "PNG");
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_SetTextureBlendMode(image.texture(), SDL_BLENDMODE_BLEND);

    float alpha = 255.0f / static_cast<float>(FadeFrames);

    for (int frame = 0; frame < FadeFrames; frame++) {
        gameRenderer->startFrameTimer();

        SDL_Rect dest;

        SDL_RenderClear(renderer);

        handleEvents();

        dest.x = (displayWidth / 2) - (image.width() / 2);
        dest.y = (displayHeight / 2) - (image.height() / 2);
        dest.w = image.width();
        dest.h = image.height();

        auto alphaValue = static_cast<uint8_t>(round(alpha * static_cast<float>(frame)));

        SDL_SetTextureAlphaMod(image.texture(), alphaValue);

        SDL_RenderCopy(renderer, image.texture(), nullptr, &dest);

        SDL_RenderPresent(renderer);

        SDL_UpdateWindowSurface(window);

        gameRenderer->endFrameTimer();
        gameRenderer->waitForNextFrame();
    }

    for (int frame = 0; frame < 60; frame++) {
        gameRenderer->startFrameTimer();

        handleEvents();

        SDL_UpdateWindowSurface(window);

        gameRenderer->endFrameTimer();
        gameRenderer->waitForNextFrame();
    }

    for (int frame = 0; frame < FadeFrames; frame++) {
        gameRenderer->startFrameTimer();

        SDL_Rect dest;

        SDL_RenderClear(renderer);

        handleEvents();

        dest.x = (displayWidth / 2) - (image.width() / 2);
        dest.y = (displayHeight / 2) - (image.height() / 2);
        dest.w = image.width();
        dest.h = image.height();

        auto alphaValue = static_cast<uint8_t>(round(alpha * static_cast<float>(FadeFrames - frame - 1)));

        SDL_SetTextureAlphaMod(image.texture(), alphaValue);

        SDL_RenderCopy(renderer, image.texture(), nullptr, &dest);

        SDL_RenderPresent(renderer);

        SDL_UpdateWindowSurface(window);

        gameRenderer->endFrameTimer();
        gameRenderer->waitForNextFrame();
    }

    for (int frame = 0; frame < 30; frame++) {
        gameRenderer->startFrameTimer();

        handleEvents();

        SDL_UpdateWindowSurface(window);

        gameRenderer->endFrameTimer();
        gameRenderer->waitForNextFrame();
    }
}
