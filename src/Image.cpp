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

#include "Image.h"

#include "FS.h"

#include <Magick++.h>
#include <SDL2/SDL.h>
#include <cassert>

Nedrysoft::Image::Image(SDL_Renderer *renderer, const std::string &fileName, const std::string &fileType) {
    Magick::Image image;

    image.magick("PNG");

    image.read(FS::BLOB(fileName));

    m_width = static_cast<int>(image.size().width());
    m_height = static_cast<int>(image.size().height());

    Magick::Pixels pixelData(image);

    auto sourcePixels = reinterpret_cast<uint8_t *>(pixelData.get(0, 0,  image.size().width(),  image.size().height()));

    m_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        static_cast<int>(image.size().width()),
        static_cast<int>(image.size().height()));

    uint8_t *destinationPixels;
    int destinationStride;

    SDL_LockTexture(m_texture, nullptr, reinterpret_cast<void **>(&destinationPixels), &destinationStride);

    assert(destinationStride == image.size().width() * sizeof(uint32_t));

    memcpy(destinationPixels, sourcePixels, image.size().width() * image.size().height() * sizeof(uint32_t));

    SDL_UnlockTexture(m_texture);

    SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
}

auto Nedrysoft::Image::texture() -> SDL_Texture * {
    return m_texture;
}

auto Nedrysoft::Image::width() const -> int {
    return m_width;
}

auto Nedrysoft::Image::height() const -> int {
    return m_height;
}
