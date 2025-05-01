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

#include "GameRenderer.h"

#if defined(TARGET_WINDOWS)
#include <windows.h>

void usleep(__int64 microSeconds) {
    HANDLE timerHandle;
    LARGE_INTEGER dueTime;

    dueTime.QuadPart = -(10 * microSeconds);

    timerHandle = CreateWaitableTimer(nullptr, TRUE, nullptr);

    SetWaitableTimer(timerHandle, &dueTime, 0, nullptr, nullptr, 0);

    WaitForSingleObject(timerHandle, INFINITE);

    CloseHandle(timerHandle);
}
#else
#include <unistd.h>
#endif

auto constexpr FramesPerSecondAverageCount = 15;

auto Nedrysoft::GameRenderer::getInstance() -> GameRenderer * {
    static GameRenderer instance;

    return &instance;
}

auto Nedrysoft::GameRenderer::initialise(
        SDL_Window *window,
        SDL_Renderer *renderer,
        int viewportWidth,
        int viewportHeight,
        float viewportScale ) -> void {

    m_renderer = renderer;
    m_window = window;
    m_viewportWidth = viewportWidth;
    m_viewportHeight = viewportHeight;
    m_viewportScale = viewportScale;
    m_startTick = 0;
    m_endTick = 0;
}

auto Nedrysoft::GameRenderer::setDefaultFont(TTF_Font *font) -> void {
    m_defaultFont = font;
}

auto Nedrysoft::GameRenderer::defaultFont() const -> TTF_Font * {
    return m_defaultFont;
}

auto Nedrysoft::GameRenderer::viewportWidth() const -> int {
    return m_viewportWidth;
}

auto Nedrysoft::GameRenderer::viewportSize() const -> Nedrysoft::Size {
    return {static_cast<float>(m_viewportWidth), static_cast<float>(m_viewportHeight)};
}

auto Nedrysoft::GameRenderer::viewportHeight() const -> int {
    return m_viewportHeight;
}

auto Nedrysoft::GameRenderer::viewportScale() const -> float {
    return m_viewportScale;
}

auto Nedrysoft::GameRenderer::renderer() -> SDL_Renderer * {
    return m_renderer;
}

auto Nedrysoft::GameRenderer::window() -> SDL_Window * {
    return m_window;
}

auto Nedrysoft::GameRenderer::averageFramesPerSecond() const -> float {
    return m_averageFramesPerSecond;
}

auto Nedrysoft::GameRenderer::averageFrameDuration() const -> float {
    return m_averageFrameDuration;
}

auto Nedrysoft::GameRenderer::beginRendering() -> void {
    //SDL_SetRenderTarget(m_renderer, renderTexture);

    m_startTick = SDL_GetTicks64();
}

auto Nedrysoft::GameRenderer::endRendering() -> void {
    m_endTick = SDL_GetTicks();

    m_frameDurations.push_back(m_endTick - m_startTick);

    m_averageFramesPerSecond = 0.0f;
    m_averageFrameDuration = 0.0f;

    for (auto frameDuration : m_frameDurations) {
        m_averageFramesPerSecond += 1000.0f / frameDuration;
        m_averageFrameDuration += frameDuration;
    }

    m_averageFramesPerSecond = m_averageFramesPerSecond / (static_cast<float>(m_frameDurations.size()));
    m_averageFrameDuration = m_averageFrameDuration / (static_cast<float>(m_frameDurations.size()));

    if (m_frameDurations.size() > FramesPerSecondAverageCount) {
        m_frameDurations.pop_front();
    }

    //SDL_SetRenderTarget(m_renderer, nullptr);

    //SDL_RenderCopy(m_renderer, m_renderTexture, &gameRenderRect, &windowRenderRect);

    //SDL_UpdateWindowSurface(m_window);
}

/*std::chrono::time_point<std::chrono::steady_clock> m_frameStartMicrosecond;
std::chrono::time_point<std::chrono::steady_clock> m_frameEndMicrosecond;
*/

auto Nedrysoft::GameRenderer::startFrameTimer() -> void {
    m_frameStartMicrosecond = std::chrono::high_resolution_clock::now();
}

auto Nedrysoft::GameRenderer::endFrameTimer() -> void {
    m_frameEndMicrosecond = std::chrono::high_resolution_clock::now();
}

auto Nedrysoft::GameRenderer::frameTime() -> double {
    return std::chrono::duration_cast<std::chrono::microseconds>(m_frameEndMicrosecond - m_frameStartMicrosecond).count();
}

auto Nedrysoft::GameRenderer::waitForNextFrame() -> void {
    if (frameTime() <= MicrosecondsPerFrame) {
        usleep(MicrosecondsPerFrame - frameTime());
    }
}
/*
auto frameStart = std::chrono::high_resolution_clock::now();


auto frameTime = std::chrono::high_resolution_clock::now() - frameStart;

double frameMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(frameTime).count();

if (frameMicroseconds <= MicrosecondsPerFrame) {
    usleep(MicrosecondsPerFrame - frameMicroseconds);
}*/