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

#ifndef NEDRYSOFT_SEGAINTRO_H
#define NEDRYSOFT_SEGAINTRO_H

#include <SDL2/SDL.h>

namespace Nedrysoft {

    class SegaIntro {
        private:
            struct RGB {
                struct  {
                    float r;
                    float g;
                    float b;
                } current;

                struct {
                    uint8_t r;
                    uint8_t g;
                    uint8_t b;
                } target;

                struct {
                    float r;
                    float g;
                    float b;
                } step;
            };

        public:
            static auto execute(SDL_Window *window, SDL_Renderer *renderer, int displayWidth, int displayHeight) -> void;

        private:
            static auto updateTexture(SDL_Texture *backgroundTexture, int width, int height, const uint32_t *sourcePixels, const uint32_t *palette) -> void;

            static auto renderFrame(SDL_Window *window, SDL_Texture *foregroundTexture, SDL_Texture *backgroundTexture) -> void;

            static auto fadeTo(const uint32_t from[64], const uint32_t to[64], int totalFrames, RGB *fadePalette) -> void;

            static auto executeFadeStep(uint32_t *targetPalette, RGB *fadePalette, int start, int stop) -> void;
    };
}


#endif //NEDRYSOFT_SEGAINTRO_H
