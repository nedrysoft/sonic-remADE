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

#include "DebugManager.h"

#include "GameRenderer.h"
#include "Utils.h"

#include <cassert>
#include <iostream>

constexpr auto MessageDisplayTicks = 1000;
constexpr auto MessageGapY = 4;
constexpr auto MessageBorderY = 4;
constexpr auto MessageBorderX = 4;

Nedrysoft::DebugMessage::DebugMessage(std::string identifier, uint64_t removalTick, SDL_Texture *texture, int width, int height) :
        m_removalTick(removalTick),
        m_texture(texture),
        m_width(width),
        m_height(height) {

     m_identifier = std::move(identifier);
}

Nedrysoft::DebugMessage::~DebugMessage() {
    SDL_DestroyTexture(m_texture);
}

auto Nedrysoft::DebugMessage::texture() -> SDL_Texture * {
    return m_texture;
}

auto Nedrysoft::DebugMessage::width() const -> int {
    return m_width;
}

auto Nedrysoft::DebugMessage::height() const -> int {
    return m_height;
}

auto Nedrysoft::DebugMessage::removalTick() const -> uint64_t {
    return m_removalTick;
}

auto Nedrysoft::DebugMessage::identifier() -> std::string {
    return m_identifier;
}

Nedrysoft::DebugManager::DebugManager() = default;

auto Nedrysoft::DebugManager::getInstance() -> DebugManager * {
    static DebugManager instance;

    return &instance;
}

auto Nedrysoft::DebugManager::registerToggle(const std::string &identifier, const std::string &name, int scancode, bool defaultValue) -> void {
    auto toggle = new DebugToggle;

    toggle->name = name;
    toggle->value = defaultValue;

    m_identifierMap[identifier] = toggle;
    m_keyMap[scancode] = toggle;

    auto keyCode = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(scancode));
    auto keyName = SDL_GetKeyName(keyCode);

    std::cout << color::rize("[debug option]", "Yellow", "Default", "Bold") +
                             debugValue("key", keyName, "Cyan", "Green", "Bold") << 
                             debugValue("toggles", name, "Cyan", "Green", "Bold") << 
     std::endl;
}

auto Nedrysoft::DebugManager::on(const std::string &identifier) -> bool {
    if (!m_identifierMap.count(identifier)) {
        return false;
    }

    return m_identifierMap[identifier]->value;
}

auto Nedrysoft::DebugManager::toggle(int scancode) -> void {
    if (m_keyMap.count(scancode)) {
        m_keyMap[scancode]->value = !m_keyMap[scancode]->value;

        auto displayString = m_keyMap[scancode]->name + " [ " + (m_keyMap[scancode]->value ? "On" : "Off") +" ]";

        addText(m_keyMap[scancode]->name, displayString, {255, 255, 255, 255});
    }
}

auto Nedrysoft::DebugManager::render() -> void {
    SDL_Rect rect;

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    rect.y = MessageBorderY;

    auto it = m_messages.begin();

    while (it != m_messages.end()) {
        rect.x = gameRenderer->viewportWidth() - MessageBorderX - (*it)->width();

        rect.w = (*it)->width();
        rect.h = (*it)->height();

        SDL_RenderCopy(gameRenderer->renderer(), (*it)->texture(), nullptr, &rect);

        rect.y += (*it)->height() + MessageGapY;

        if (SDL_GetTicks64() > (*it)->removalTick()) {
            delete *it;

            it = m_messages.erase(it);
        } else {
            ++it;
        }
    }
}

auto Nedrysoft::DebugManager::removeAllMessages(const std::string &identifier) -> void {
    auto it = m_messages.begin();

    while (it != m_messages.end()) {
       if ((*it)->identifier() == identifier) {
            delete *it;

            it = m_messages.erase(it);
        } else {
            ++it;
        }
    }
}

auto Nedrysoft::DebugManager::addText(const std::string &identifier, const std::string &text, SDL_Color colour) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    //TTF_SetFontOutline(gameRenderer->defaultFont(), 2);

    auto surface = TTF_RenderText_Blended(gameRenderer->defaultFont(), text.c_str(), colour);

    auto texture = SDL_CreateTextureFromSurface(gameRenderer->renderer(), surface);

    SDL_FreeSurface(surface);

    int width;
    int height;

    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    auto removalTick = SDL_GetTicks64() + MessageDisplayTicks;

    removeAllMessages(identifier);

    m_messages.emplace_back(new DebugMessage(identifier, removalTick, texture, width, height));
}

auto Nedrysoft::DebugManager::beginMetrics(const std::string &id) -> void {
    m_startTicks[id] = SDL_GetTicks64();
    m_endTicks[id] = -1;
}

auto Nedrysoft::DebugManager::endMetrics(const std::string &id) -> void {
    m_endTicks[id] = SDL_GetTicks64();
}

auto Nedrysoft::DebugManager::ticks(const std::string &id) -> int {
    if (id.empty()) {
        int ticks = 0;

        for (auto &value : m_endTicks) {
            assert(m_startTicks.count(value.first));

            ticks += static_cast<int>(value.second - m_startTicks[value.first]);
        }

        return ticks;
    }

    return static_cast<int>(m_endTicks[id] - m_startTicks[id]);
}

