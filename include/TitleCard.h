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

#ifndef NEDRYSOFT_TITLECARD_H
#define NEDRYSOFT_TITLECARD_H

#include "Image.h"
#include "Structs.h"

#include <map>

namespace Nedrysoft {
    class GameRenderer;

    struct CardData {
        int start;
        int end;
        int sign;
    };

    enum TitleCardZone {
        GreenHill,
        Labyrinth,
        Marble,
        ScrapBrain,
        SpringYard,
        StarLight
    };

    enum TitleCardAct {
        One = StarLight + 1,
        Two,
        Three
    };

    enum TitleCardImage {
        Level,
        Zone,
        Act,
        Oval
    };

    enum TitleCardState {
        In,
        PreFade,
        Fade,
        PostFade,
        Out,
        Finished
    };

    class TitleCard {
        public:
            TitleCard();

            auto start(TitleCardZone zone, TitleCardAct act) -> void;

            auto render() -> void;
            auto update() -> void;

            auto renderImage(int id) -> void;

            auto finished() -> bool;

        private:

            std::map<int, Image *> m_levelImages;
            std::map<int, Image *> m_actImages;

            Image *m_zoneImage;
            Image *m_ovalImage;

            int m_zoneId;
            int m_actId;
            int m_alpha;

            int m_positions[4];
            CardData m_cardData[4];

            int m_timer;

            static int m_imageY[4];

            TitleCardState m_state;
    };
}

#endif //NEDRYSOFT_TITLECARD_H
