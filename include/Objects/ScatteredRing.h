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

#ifndef NEDRYSOFT_SCATTEREDRING_H
#define NEDRYSOFT_SCATTEREDRING_H

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Object.h"

namespace Nedrysoft {
    class Camera;
    class Sonic;

    /**
     * @brief           This object is spawned dynamically when Sonic is hurt, the scattered ring has a limited life time
     *                  and if not re-collected it will automatically disappear.  There are additional rules that are also
     *                  in place that prevent sonic from collecting the ring too early after scattering.
     */

    class ScatteredRing :
            public Object {

        private:
            enum class RingState {
                Bouncing,
                Sparkling,
                None
            };

        public:
            ScatteredRing(float x, float y, float xSpeed, float ySpeed);

            /**
             * @brief           Initialises the static object.
             * 
             * @details         Object instances may share other objects or data rather than duplicate them, this function is called once in the lifetime
             *                  of the engine before any instances are created.
             */ 
            static auto initialise() -> void;

            /**
             * @brief           Updates static object data.
             * 
             * @details         This function is called once per game loop to allow any static data to be upated.
             */
            static auto update() -> void;

            /** 
             * @copydoc         Nedrysoft::Object::render(Camera *, ObjectRenderPriority)
             */
            auto render(Camera *camera, ObjectRenderPriority priority) -> void override;

            /** 
             * @copydoc         Nedrysoft::Object::update(TileMap *, Camera *, Sonic *)
             */
            auto update(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool override;

            /**
             * @copydoc         Nedrysoft::Object::spawn(Camera *)
             */
            auto spawn(Camera *camera) -> bool override;

            /**
             * @copydoc         Nedrysoft::Object::reset()
             */
            auto reset() -> void override;

        private:

            AnimationPlayer m_animation;

            int m_lifespanTimer;                        //<! Started when the ring is spawned, when zero it's deleted.
            int m_SparklingTimer;                       //<! This timer ticks down after being collected, once elapsed the ring is deleted.
            int m_collectionTimer;                      //<! This timer ticks down after spawning, once elapsed ring can then be collected.

            float m_xSpeed;
            float m_ySpeed;

            RingState m_state;
    };
}

#endif //NEDRYSOFT_SCATTEREDRING_H
