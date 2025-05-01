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

#ifndef NEDRYSOFT_BUZZBOMBER_H
#define NEDRYSOFT_BUZZBOMBER_H

#include "Object.h"
#include "AnimationPlayer.h"

#include <SDL2/SDL.h>

namespace Nedrysoft {
    enum BuzzBomberState {
        Flying,
        PreparingToFire,
        Firing,
        PreparingToMove,
        Turning
    };

    class BuzzBomber :
            public Object {

        public:
            /**
             * @brief           Constructs a new buzz bomber object.
             *
             * @param[in]       subType the subtype.
             * @param[in]       x the X coordinate of the object.
             * @param[in]       y the Y coordinate of the object.
             * @param[in]       rememberState whether the state is remembered when spawned again.
             * @param[in]       mirrored whether the object is mirrored in x.
             * @param[in]       mirrored whether the object is flipped in y.
             */
            BuzzBomber(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped);

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
            auto updateFlying(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool;
            auto updateFiring(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool;
            auto updateTurning(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool;
            auto updatePreparingToMove(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool;
            auto updatePreparingToFire(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> bool;

            auto setState(int state, class Sonic *sonic = nullptr) -> int;
            auto checkCollision(class Sonic *sonic) -> bool;

        private:

            AnimationPlayer m_animation;

            float m_xSpeed;
            float m_xPosition;

            bool m_alive;
            bool m_firingLock;

            int m_state;
            int m_timer;
    };
}

#endif //NEDRYSOFT_BUZZBOMBER_H
