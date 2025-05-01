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

#ifndef NEDRYSOFT_ANIMATIONS_H
#define NEDRYSOFT_ANIMATIONS_H

#include "Structs.h"
#include "AnimationPlayer.h"

#include <Magick++.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace Nedrysoft {
    class AnimationStep;
    class AnimationPlayer;
    class Animation;
    class AnimationSprite;

    class Animations {
        public:
            static auto load(SDL_Renderer *renderer, const std::string &filename) -> Nedrysoft::Animations *;

            auto sprite(const std::string &id) -> AnimationSprite *;
            auto sprite(int index) -> AnimationSprite *;

            auto find(const std::string &name) -> Animation *;

            auto start(const std::string &name, AnimationPlayer *runningAnimation, int frameDuration = -1) -> bool;
            auto start(const std::string &name, AnimationPlayer *runningAnimation, const FrameDurationFunction &durationFunction) -> bool;
            auto start(const std::list<std::string> &group, AnimationPlayer *runningAnimation, const FrameDurationFunction &durationFunction) -> bool;

            auto index(const std::string &id) -> int;

        private:
            Animations() = default;
            ~Animations() = default;

        private:
            std::vector<AnimationSprite *> m_sprites;

            std::map<std::string, AnimationSprite *> m_spriteMap;

            std::vector<Animation *> m_animations;
    };
}

#endif //NEDRYSOFT_ANIMATIONS_H
