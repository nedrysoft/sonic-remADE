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

#ifndef NEDRYSOFT_HUD_H
#define NEDRYSOFT_HUD_H

#include "Image.h"

namespace Nedrysoft {
    class Sonic;

    enum class HudColour {
        White = 0,
        Yellow = 1,
        Red = 2
    };

    /**
     * @brief       The HUD class provides the overlay displaying the current status of the playing game.
     */
    class Hud {
        public:
            Hud();

            static auto getInstance() -> Hud *;

            auto render(class Nedrysoft::Sonic *sonic) -> void;

            auto reset() -> void;

        private:
            auto initialise() -> void;

            static auto drawSprite(Image *image, int x, int y, HudColour colour) -> void;
            auto drawNumber(int value, int digits, int x, int y, HudColour colour) -> void;
            auto drawTime(int seconds, int x, int y, HudColour colour) -> void;
            auto drawLivesNumber(int value, int digits, int x, int y, HudColour colour) -> void;

        private:
            Image *m_numbersImage;
            Image *m_livesCounterNumbersImage;
            Image *m_ringsImage;
            Image *m_scoreImage;
            Image *m_sonicLivesImage;
            Image *m_timeImage;

            int m_framesElapsed;
            int m_lives;
            int m_rings;
            int m_score;
    };
}

#endif //NEDRYSOFT_HUD_H
