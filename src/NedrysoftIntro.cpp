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

#include "NedrysoftIntro.h"

#include "GameRenderer.h"
#include "FS.h"

#include <SDL_image.h>
#include <SDL_mixer.h>

#ifdef TARGET_WINDOWS
#include <windows.h>
#define sleep(milliseconds) Sleep(milliseconds)
#else
#include <unistd.h>
#endif

#define TotalLines                          32

#define GlitchNormal                        7
#define GlitchMultiplier                    9
#define GlitchMaxLines                      10

#define NedrysoftSillyRandMax               0x7fffffff
#define NedrysoftSillyRandMultiplier        1103515245
#define NedrysoftSillyRandIncrementor       12345

auto handleEvents() -> bool;

auto Nedrysoft::NedrysoftIntro::execute(SDL_Window *window, SDL_Renderer *renderer, int displayWidth, int displayHeight) -> void {
    int frameCount = 0;

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    /**
     * we use our own random function which allows us to seed with the same number and get the same results every
     * single run.  We do this as it allows us to find a seed that produces an aesthetically pleasing "run" that
     * is the same every single time, the seed is chosen such that it creates an end result that fits exactly
     * hpw we want it to look.
     */

    SDL_Texture *texture = IMG_LoadTexture(renderer, FS::toNative("./data/art/misc/nedrysoft.png").c_str());

    Mix_Chunk *sound = Mix_LoadWAV(FS::toNative("./data/sound/sfx/nedrysoft.wav").c_str());

    SDL_Rect dest;

    SDL_QueryTexture(texture, nullptr, nullptr, &dest.w, &dest.h);

    sleep(1);

    Mix_PlayChannel(-1, sound, 0);

    while(frameCount < (5 * 60) + 10) {
        gameRenderer->startFrameTimer();

        /**
         * generate a random number between 0 and 10, if it's between 0 and 7 (inclusive) then we do the normal effect,
         * the rest of the time we clear the scroll registers and the bitmap is shown normal, this allows a few frames to show the
         * logo as normal, and give the effect of something glitching.
         */

        uint32_t imageX = (displayWidth / 2) - (dest.w / 2);
        uint32_t imageY = (displayHeight / 2) - (dest.h / 2);

        if ((sillyRand() % 10) > 7) {
            dest.x = (int) imageX;
            dest.y = (int) imageY;

            /**
             * when we show the normal image, we keep it on screen for 10, 20 or 30 frames.
             */

            for (uint32_t i = 0;i < ((sillyRand() % 2) + 1) * 10; i++) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);

                handleEvents();

                SDL_RenderCopy(renderer, texture, nullptr, &dest);

                SDL_RenderPresent(renderer);

                SDL_UpdateWindowSurface(window);

                frameCount++;
            }
        } else {
            uint32_t glitchLine = 0;

            /**
             * not a normal frame, so we now glitch the image, we choose a number number of lines and glitch those with the same value,
             * and repeat until we've filled the scroll array up with values
             */

            uint32_t glitchValues[32];

            while(glitchLine < TotalLines) {
                uint32_t glitchIndex = 0;
                uint32_t glitchHeight = (sillyRand() % GlitchMaxLines);
                uint32_t glitchOffset = (sillyRand() % GlitchNormal) - (GlitchNormal / 2);

                /**
                 * normally we glitch by a small amount, we bias the offset to be in that range, but we occasionally multiply the glitch offset
                 * to produce a more dramatic offset for some lines
                 */

                if ((sillyRand() % 5) > 3) {
                    glitchOffset *= GlitchMultiplier;
                }

                /**
                 * right at the end we permit the glitch to be even bigger for a nicer effect
                 */

                if (frameCount >= (2 * 60)) {
                    glitchOffset *= 3;
                }

                while((glitchIndex < glitchHeight) && (glitchLine < TotalLines)) {
                    glitchValues[glitchLine++] = imageX - glitchOffset;

                    glitchIndex++;
                }

                glitchLine++;
            }

            /**
             * we keep the "super glitched" image on the screen for 0 to 14 frames
             */

            for (uint32_t i = 0; i < sillyRand() % 15; i++) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);

                handleEvents();

                SDL_Rect glitchSourceRect;
                SDL_Rect glitchDestRect;

                for (int y = 0; y < 32; y++) {
                    glitchSourceRect.x = 0;
                    glitchSourceRect.y = y;
                    glitchSourceRect.w = dest.w;
                    glitchSourceRect.h = 1;

                    glitchDestRect.x = (int) glitchValues[y];
                    glitchDestRect.y = (int) imageY + y;
                    glitchDestRect.w = dest.w;
                    glitchDestRect.h = 1;

                    SDL_RenderCopy(renderer, texture, &glitchSourceRect, &glitchDestRect);
                }

                SDL_RenderPresent(renderer);

                SDL_UpdateWindowSurface(window);

                frameCount++;
            }
        }

        frameCount++;

        gameRenderer->endFrameTimer();
        gameRenderer->waitForNextFrame();
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);

    SDL_UpdateWindowSurface(window);

    sleep(4);
}

uint32_t Nedrysoft::NedrysoftIntro::sillyRand() {
    static uint32_t next = 666666;

	return ((next = next * NedrysoftSillyRandMultiplier + NedrysoftSillyRandIncrementor) % ((uint32_t) NedrysoftSillyRandMax + 1));
}
