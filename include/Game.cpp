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

#include "Game.h"

#include "Audio.h"
#include "Errors.h"
#include "GameRenderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <variant>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
#pragma ide diagnostic ignored "Simplify"

auto Nedrysoft::Game::getInstance() -> Game * {
    static Game instance;

    return &instance;
}

auto Nedrysoft::Game::eventHandler() -> void {

}

auto Nedrysoft::Game::initialise() -> bool {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
		std::cout << "Couldn't initialize SDL: " << SDL_GetError() << std::endl;

		return Nedrysoft::ErrorCodes::SDLInitialisationFailed;
	}

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        std::cout << "Couldn't initialize SDL Mixer: " << SDL_GetError() << std::endl;

		return Nedrysoft::ErrorCodes::SDLMixerInitialisationFailed;
    }

    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    Mix_AllocateChannels(2);

    auto audio = Nedrysoft::Audio::getInstance();

    return ErrorCodes::ErrorCode::Ok;
}

auto Nedrysoft::Game::initialiseRenderer(int width, int height, float scaleFactor) -> ErrorCodes::ErrorCode {
    m_size = ISize(width, height);
    m_scaleFactor = scaleFactor;

    m_windowRenderRect.x = 0;
    m_windowRenderRect.y = 0;

    m_windowRenderRect.w = static_cast<int>(static_cast<float>(width) * scaleFactor);
    m_windowRenderRect.h = static_cast<int>(static_cast<float>(height) * scaleFactor);

    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "0");

    auto window = SDL_CreateWindow(
	    "Whitechapel",
	    SDL_WINDOWPOS_UNDEFINED,
	    SDL_WINDOWPOS_UNDEFINED,
	    m_windowRenderRect.w,
        m_windowRenderRect.h,
	    0//SDL_WINDOW_FULLSCREEN
	);

    if (!window) {
        return Nedrysoft::ErrorCodes::SDLCreateWindowFailed;
    }

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    auto windowSurface = SDL_GetWindowSurface(window);

    if (!windowSurface) {
        return Nedrysoft::ErrorCodes::SDLGetWindowSurfaceFailed;
    }

    auto renderer = SDL_CreateSoftwareRenderer(windowSurface);

    if (!renderer) {
        return Nedrysoft::ErrorCodes::SDLCreateSoftwareRendererFailed;
    }

    if (!renderer) {
        return Nedrysoft::ErrorCodes::SDLCreateSoftwareRendererFailed;
    }

    m_gameRenderRect.x = 0;
    m_gameRenderRect.y = 0;
    m_gameRenderRect.w = m_size.width();
    m_gameRenderRect.h = m_size.height();

    m_renderTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_TARGET,
        m_gameRenderRect.w,
        m_gameRenderRect.h
    );

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_ShowCursor(0);

    auto font = TTF_OpenFont("./data/fonts/Born2bSportyV2.ttf", 16);

    if (!font) {
        std::cout << "Unable to load font: " << SDL_GetError() << std::endl;

        return Nedrysoft::ErrorCodes::SDLLoadingFontFailed;
    }

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    gameRenderer->initialise(window, renderer, m_size.width(), m_size.height(), m_scaleFactor);
    gameRenderer->setDefaultFont(font);

    return ErrorCodes::ErrorCode::Ok;
}

#pragma clang diagnostic pop