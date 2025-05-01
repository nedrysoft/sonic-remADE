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

#ifndef NEDRYSOFT_DEBUGMANAGER_H
#define NEDRYSOFT_DEBUGMANAGER_H

#ifdef NEDRYSOFT_DEBUG
#define DebugManagerEnabled
#define DebugManager_IsOn(setting, state) Nedrysoft::DebugManager::getInstance()->on(setting)
#define DebugManager_BeginMetrics(id) Nedrysoft::DebugManager::getInstance()->beginMetrics(id)
#define DebugManager_EndMetrics(id) Nedrysoft::DebugManager::getInstance()->endMetrics(id)
#else
#define DebugManager_IsOn(setting, state) state
#define DebugManager_BeginMetrics(id)
#define DebugManager_EndMetrics(id)
#endif

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_render.h>

#include <iostream>
#include <list>
#include <map>
#include <string>

namespace Nedrysoft {
    struct DebugToggle {
        std::string name;
        bool value;
    };

    class DebugMessage {
        public:
            DebugMessage(std::string identifier, uint64_t removalTick, SDL_Texture *texture, int width, int height);

            ~DebugMessage();

            auto texture() -> SDL_Texture *;

            [[nodiscard]] auto width() const -> int;

            [[nodiscard]] auto height() const -> int;

            [[nodiscard]] auto removalTick() const -> uint64_t;

            auto identifier() -> std::string;

        private:
            uint64_t m_removalTick;
            SDL_Texture *m_texture;
            int m_width;
            int m_height;
            std::string m_identifier;
    };

    class DebugManager {
        public:
            auto static getInstance() -> DebugManager *;

            auto registerToggle(const std::string &identifier, const std::string &name, int scancode, bool defaultValue) -> void;

            auto on(const std::string &identifier) -> bool;

            auto toggle(int scancode) -> void;

            auto render() -> void;

            auto addText(const std::string &identifier, const std::string &text, SDL_Color colour = {255, 255, 255, 0}) -> void;

            auto beginMetrics(const std::string &id) -> void;
            auto endMetrics(const std::string &id) -> void;

            auto ticks(const std::string &id = "") -> int;

        private:
            DebugManager();
            ~DebugManager() = default;

            auto removeAllMessages(const std::string &type) -> void;

        private:
            std::map<std::string, DebugToggle *> m_identifierMap;
            std::map<int, DebugToggle *> m_keyMap;

            std::list<DebugMessage *> m_messages;

            std::map<std::string, uint64_t> m_startTicks;
            std::map<std::string, uint64_t> m_endTicks;
    };
}

#endif //NEDRYSOFT_DEBUGMANAGER_H