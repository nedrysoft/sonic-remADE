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

#ifndef NEDRYSOFT_TITLESCREEN_H
#define NEDRYSOFT_TITLESCREEN_H

#include "Animations.h"
#include "AnimationPlayer.h"

#include <vector>

namespace Nedrysoft {
    class Image;
    class Animations;
    class AnimationPlayer;

    enum class TitleScreenState {
        Idle,
        FadeIn,
        Normal,
        FadeOut,
        Finished
    };

    class TitleScreen {

        public:
            TitleScreen();

            auto render() -> void;
            auto reset() -> void;

            auto finished() -> bool;
            auto start() -> void;
            auto end() -> void;

        private:
            Image *m_foreground;
            Image *m_pushStartButton;

            Animations *m_animations;
            AnimationPlayer m_sonicAnimation;

            int m_y;
            int m_frame;
            int m_alpha;

            TitleScreenState m_state;
    };
}


#endif //NEDRYSOFT_TITLESCREEN_H
