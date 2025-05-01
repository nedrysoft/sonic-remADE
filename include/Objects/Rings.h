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

#ifndef NEDRYSOFT_RINGS_H
#define NEDRYSOFT_RINGS_H

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Object.h"

namespace Nedrysoft {
    class Camera;
    class Sonic;

    enum class RingState {
        Uncollected,
        Sparkling,
        Collected
    };

    struct RingStateInfo {
        AnimationPlayer animation;
        RingState state = RingState::Uncollected;
        int timer{};
    };

    class Rings :
            public Object {

        public:
            /**
             * @brief           Constructs a new rings object.
             *
             * @param[in]       subType the subtype.
             * @param[in]       x the X coordinate of the object.
             * @param[in]       y the Y coordinate of the object.
             * @param[in]       rememberState whether the state is remembered when spawned again.
             * @param[in]       mirrored whether the object is mirrored in x.
             * @param[in]       mirrored whether the object is flipped in y.
             */
            Rings(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped);

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
             * @copydoc         Nedrysoft::Object::renderCollision(Camera *)
             */
            auto renderCollision(Camera *camera) -> void override;

            /**
             *  @copydoc        Nedrysoft::Object::reset()
             */
            auto reset() -> void override;

        private:

            static AnimationPlayer m_ringAnimation;

            RingStateInfo m_states[0x0F];

            int m_ringsRemaining;

    };
}

#endif //NEDRYSOFT_RINGS_H
