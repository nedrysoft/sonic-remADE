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

#ifndef NEDRYSOFT_ANIMATION_H
#define NEDRYSOFT_ANIMATION_H

#include <string>
#include <vector>

namespace Nedrysoft {
    class AnimationStep;

    class AnimationStep;
    class AnimationPlayer;
    
    /**
     * @brief           The animation class is used to load and provide the necessary functions so that the
     *                  animation player class can use to show the animation.
     */

    class Animation {
        public:
            Animation(std::string name, int speed, std::vector<AnimationStep *> script);

            Animation();

            auto name() -> std::string;

            [[nodiscard]] auto speed() const -> int;

            auto script() -> std::vector<AnimationStep *> &;

            auto setName(std::string name) -> void;

            auto setSpeed(int speed) -> void;

            auto setScript(std::vector<AnimationStep *> script) -> void;

            auto start() -> AnimationStep *;

        private:

            int m_speed;

            std::string m_name;

            std::vector<AnimationStep *> m_script;
    };
}

#endif //NEDRYSOFT_ANIMATION_H
