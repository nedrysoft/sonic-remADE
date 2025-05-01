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

#include "SegaIntro.h"

#include "GameRenderer.h"
#include "FS.h"
#include "Utils.h"

#include <Magick++.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#if !defined(TARGET_WINDOWS)
#include <unistd.h>
#endif
#include <cstdint>

auto constexpr BackgroundPositionX = 8 * 8;
auto constexpr BackgroundPositionY = 10 * 8;
auto constexpr FadeFrames = 10;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"

auto constexpr ORGB(uint32_t r, uint32_t g, uint32_t b) -> uint32_t {
    return 0xFF000000 | (static_cast<uint8_t>(r) << 16) | (static_cast<uint8_t>(g) << 8) | (static_cast<uint8_t>(b));
}

auto constexpr ARGB(uint32_t a, uint32_t r, uint32_t g, uint32_t b) -> uint32_t {
    return (static_cast<uint8_t>(0) << 24) | (static_cast<uint8_t>(r) << 16) | (static_cast<uint8_t>(g) << 8) | (static_cast<uint8_t>(b));
}
#pragma clang diagnostic pop

static const uint32_t AnimationPalette[6] = {
    ORGB(0xFF, 0xFF, 0xFF),
    ORGB(0x96, 0xFF, 0xFF),
    ORGB(0x00, 0xFF, 0xFF),
    ORGB(0x01, 0xD2, 0xFF),
    ORGB(0x00, 0xFF, 0xFF),
    ORGB(0x96, 0xFF, 0xFF)
};

static const uint32_t FadePaletteGradient[7] = {
    ARGB(0x00, 0x00, 0x00, 0x00),
    ORGB(0xFF, 0xFF, 0xFF),
    ORGB(0x96, 0xFF, 0xFF),
    ORGB(0x00, 0xFF, 0xFF),
    ORGB(0x01, 0xD2, 0xFF),
    ORGB(0x00, 0xB2, 0xFF),
    ORGB(0x01, 0x94, 0xFF),
};

static const uint32_t FadePaletteSolid[16] = {
    ARGB(0x00, 0x00, 0x00, 0x00),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF),
    ORGB(0x0B, 0x00, 0xFF)
};

auto handleEvents() -> bool;

auto Nedrysoft::SegaIntro::execute(SDL_Window *window, SDL_Renderer *renderer, int displayWidth, int displayHeight) -> void {
    N_UNUSED(renderer)
    N_UNUSED(displayWidth)
    N_UNUSED(displayHeight)

    Magick::Image backgroundImage;
    Magick::Image foregroundImage;

    uint32_t whitePalette[64];
    uint32_t blackPalette[64];
    uint32_t palette[64];

    for (auto &entry : whitePalette) {
        entry = ORGB(0xFF, 0xFF, 0xFF);
    }

    for (auto &entry : blackPalette) {
        entry = ORGB(0x00, 0x00, 0x00);
    }

    /**
     * foreground SEGA logo
     */

    foregroundImage.magick("PNG");

    foregroundImage.read(FS::BLOB("./data/art/objects/sega/sega-fg.png"));

    Magick::Pixels foregroundPixelData(foregroundImage);

    auto foregroundWidth = static_cast<int>(foregroundImage.size().width());
    auto foregroundHeight = static_cast<int>(foregroundImage.size().height());

    auto foregroundPixels = reinterpret_cast<uint32_t *>(foregroundPixelData.get(0, 0,  foregroundImage.size().width(),  foregroundImage.size().height()));

    auto gameRenderer = GameRenderer::getInstance();

    auto foregroundTexture = SDL_CreateTexture(gameRenderer->renderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, foregroundWidth, foregroundHeight);

    SDL_SetTextureBlendMode(foregroundTexture, SDL_BLENDMODE_BLEND);

    /**
     * background gradient image
     */

    backgroundImage.magick("PNG");

    backgroundImage.read(FS::BLOB("./data/art/objects/sega/sega-bg.png"));

    Magick::Pixels backgroundPixelData(backgroundImage);

    auto backgroundWidth = static_cast<int>(backgroundImage.size().width());
    auto backgroundHeight = static_cast<int>(backgroundImage.size().height());

    auto backgroundPixels = reinterpret_cast<uint32_t *>(backgroundPixelData.get(0, 0,  backgroundImage.size().width(),  backgroundImage.size().height()));

    auto backgroundTexture = SDL_CreateTexture(gameRenderer->renderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, backgroundWidth, backgroundHeight);

    SDL_SetTextureBlendMode(backgroundTexture, SDL_BLENDMODE_BLEND);

    /**
     * first we run the "highlight SEGA" animation...
     */

    memcpy(palette, whitePalette, sizeof(palette));

    int animationColours = sizeof(AnimationPalette) / sizeof(uint32_t);

    for(int cycleIndex = (16 - animationColours); cycleIndex < (64 + animationColours); cycleIndex++ ) {
        for (int colourIndex = 0; colourIndex < animationColours; colourIndex++) {
            if ((colourIndex + cycleIndex) < 64) {
                palette[cycleIndex + colourIndex] = AnimationPalette[colourIndex];
            } else {
                palette[cycleIndex + colourIndex] = ORGB(0xFF, 0xFF, 0xFF);
            }
        }

        palette[0] = 0x00FFFFFF;
        palette[16] = 0x00FFFFFF;
        palette[32] = 0x00FFFFFF;
        palette[48] = 0x00FFFFFF;

        updateTexture(foregroundTexture, foregroundWidth, foregroundHeight, foregroundPixels, reinterpret_cast<uint32_t *>(&palette));

        updateTexture(backgroundTexture, backgroundWidth, backgroundHeight, backgroundPixels, reinterpret_cast<uint32_t *>(&palette));

        renderFrame(window, foregroundTexture, backgroundTexture);
    }

    /**
     * Now we fade the SEGA logo into the solid blue...
     */

    uint32_t targetPalette[64];

    memset(targetPalette, 0, sizeof(targetPalette));
    memcpy(targetPalette, whitePalette, sizeof(targetPalette));

    memcpy(&targetPalette[0], FadePaletteGradient, sizeof(FadePaletteGradient));
    memcpy(&targetPalette[16], FadePaletteSolid, sizeof(FadePaletteSolid));
    memcpy(&targetPalette[32], FadePaletteSolid, sizeof(FadePaletteSolid));
    memcpy(&targetPalette[48], FadePaletteSolid, sizeof(FadePaletteSolid));

    RGB fadePalette[64];

    fadeTo(palette, targetPalette, FadeFrames, fadePalette);

    Mix_Chunk *sound = Mix_LoadWAV("./data/sound/sfx/Sega.wav");

    Mix_PlayChannel(-1, sound, 0);

    for (int frame = 0; frame < FadeFrames; frame++) {
        executeFadeStep(palette, fadePalette, 1, 64);

        updateTexture(foregroundTexture, foregroundWidth, foregroundHeight, foregroundPixels, reinterpret_cast<uint32_t *>(&palette));

        updateTexture(backgroundTexture, backgroundWidth, backgroundHeight, backgroundPixels, reinterpret_cast<uint32_t *>(&palette));

        renderFrame(window, foregroundTexture, backgroundTexture);
    }

    /**
     * we wait for 120 frames from the start of the fade to "blue", during this time the "SEGA" sound plays...
     */

    for (int frame = 0; frame < 120 - FadeFrames; frame++) {
        renderFrame(window, foregroundTexture, backgroundTexture);
    }

    /**
     * now fade to black...
     */

    memset(targetPalette, 0, sizeof(targetPalette));

    fadeTo(palette, targetPalette, FadeFrames, fadePalette);

    for (int frame = 0; frame < FadeFrames; frame++) {
        executeFadeStep(palette, fadePalette, 1, 64);

        updateTexture(foregroundTexture, foregroundWidth, foregroundHeight, foregroundPixels, reinterpret_cast<uint32_t *>(&palette));

        updateTexture(backgroundTexture, backgroundWidth, backgroundHeight, backgroundPixels, reinterpret_cast<uint32_t *>(&palette));

        renderFrame(window, foregroundTexture, backgroundTexture);
    }

    /**
     * and wait....
     */

    for (int frame = 0; frame < 60; frame++) {
        renderFrame(window, foregroundTexture, backgroundTexture);
    }
}

auto Nedrysoft::SegaIntro::updateTexture(SDL_Texture *texture, int width, int height, const uint32_t *sourcePixels, const uint32_t *palette) -> void {
    uint32_t *destinationPixels;
    int destinationStride;

    SDL_LockTexture(texture, nullptr, reinterpret_cast<void **>(&destinationPixels), &destinationStride);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            *(destinationPixels++) = palette[(sourcePixels[(y * width) + x] >> 16) & 0xFF];
        }
    }

    SDL_UnlockTexture(texture);
}

auto Nedrysoft::SegaIntro::renderFrame(SDL_Window *window, SDL_Texture *foregroundTexture, SDL_Texture *backgroundTexture) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    gameRenderer->startFrameTimer();

    auto renderer = Nedrysoft::GameRenderer::getInstance()->renderer();

    uint32_t format;int access;

    SDL_Rect dest;

    SDL_QueryTexture(backgroundTexture, &format, &access, &dest.w, &dest.h);

    SDL_RenderClear(renderer);

    handleEvents();

    dest.x = BackgroundPositionX;
    dest.y = BackgroundPositionY;

    SDL_RenderCopy(renderer, backgroundTexture, nullptr, &dest);

    SDL_QueryTexture(foregroundTexture, &format, &access, &dest.w, &dest.h);

    dest.x = 0;
    dest.y = 0;

    SDL_RenderCopy(renderer, foregroundTexture, nullptr, &dest);

    SDL_RenderPresent(renderer);

    SDL_UpdateWindowSurface(window);

    gameRenderer->endFrameTimer();
    gameRenderer->waitForNextFrame();
}

auto Nedrysoft::SegaIntro::fadeTo(const uint32_t from[64], const uint32_t to[64], int totalFrames, RGB *fadePalette) -> void {
    for (int i = 0; i < 64; i++) {
        auto startRed = static_cast<float>((from[i] >> 16) & 0xFF);
        auto startGreen = static_cast<float>((from[i] >> 8) & 0xFF);
        auto startBlue = static_cast<float>((from[i] >> 0) & 0xFF);

        auto endRed = static_cast<float>((to[i] >> 16) & 0xFF);
        auto endGreen = static_cast<float>((to[i] >> 8) & 0xFF);
        auto endBlue = static_cast<float>((to[i] >> 0) & 0xFF);

        fadePalette[i].step.r = (endRed - startRed) / static_cast<float>(totalFrames);
        fadePalette[i].step.g = (endGreen - startGreen) / static_cast<float>(totalFrames);
        fadePalette[i].step.b = (endBlue - startBlue) / static_cast<float>(totalFrames);

        fadePalette[i].current.r = static_cast<float>(startRed);
        fadePalette[i].current.g = static_cast<float>(startGreen);
        fadePalette[i].current.b = static_cast<float>(startBlue);

        fadePalette[i].target.r = (to[i] >> 16) & 0xFF;
        fadePalette[i].target.g = (to[i] >> 8) & 0xFF;
        fadePalette[i].target.b = (to[i] >> 0) & 0xFF;
    }
}

auto Nedrysoft::SegaIntro::executeFadeStep(uint32_t *targetPalette, RGB *fadePalette, int start, int stop) -> void {
    for (int paletteIndex = start; paletteIndex < stop; paletteIndex++) {
        fadePalette[paletteIndex].current.r += fadePalette[paletteIndex].step.r;
        fadePalette[paletteIndex].current.g += fadePalette[paletteIndex].step.g;
        fadePalette[paletteIndex].current.b += fadePalette[paletteIndex].step.b;

        auto red = (fadePalette[paletteIndex].current.r);
        auto green = (fadePalette[paletteIndex].current.g);
        auto blue = (fadePalette[paletteIndex].current.b);

        targetPalette[paletteIndex] = ORGB(
            static_cast<uint8_t>(red),
            static_cast<uint8_t>(green),
            static_cast<uint8_t>(blue)
        );
    }
}
