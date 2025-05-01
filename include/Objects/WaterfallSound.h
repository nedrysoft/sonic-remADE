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

#ifndef NEDRYSOFT_WATERFALLSOUND_H
#define NEDRYSOFT_WATERFALLSOUND_H

#include "Image.h"
#include "Object.h"

namespace Nedrysoft {
    class Camera;
    class Sonic;

    class WaterfallSound :
            public Object {

        public:
            /**
             * @brief           Constructs a new waterfall sound object.
             *
             * @param[in]       subType the subtype.
             * @param[in]       x the X coordinate of the object.
             * @param[in]       y the Y coordinate of the object.
             * @param[in]       rememberState whether the state is remembered when spawned again.
             * @param[in]       mirrored whether the object is mirrored in x.
             * @param[in]       mirrored whether the object is flipped in y.
             */
            WaterfallSound(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped);

            /**
             * @brief           Updates static object data.
             * 
             * @details         This function is called once per game loop to allow any static data to be upated.
             */
            static auto update() -> void;

            /**
             *  @copydoc        Nedrysoft::Object::update()
             */
            auto update(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool override;

            /**
             *  @copydoc        Nedrysoft::Object::update()
             */
            auto spawn(Camera *camera) -> bool override;

            /**
             *  @copydoc        Nedrysoft::Object::despawn()
             */
            auto despawn(Camera *camera) -> void override;

            /**
             *  @copydoc        Nedrysoft::Object::reset()
             */
            auto reset() -> void override; 

        private:
            bool m_playingSound;
    };
}

#endif //NEDRYSOFT_WATERFALLSOUND_H
