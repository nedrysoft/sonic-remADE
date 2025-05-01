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

#ifndef NEDRYSOFT_SOLID_H
#define NEDRYSOFT_SOLID_H

#include "AnimationPlayer.h"
#include "Image.h"
#include "Object.h"

#include <SDL2/SDL.h>

//#define nodiscard [[nodiscard]]

namespace Nedrysoft {
    class Camera;
    class Sonic;
    class Animations;

    enum SolidCollisionFlags {
        None,
        Top,
        Bottom,
        Left,
        Right
    };

    enum SolidCollisionStates {
        NoCollision = 0,
        CollisionOnLeft = 1,
        CollisionOnRight = 2,
        CollisionInX = 4,
        CollisionInY = 8
    };

    /**
     * @brief           The base class for a solid object.
     *
     * @details         This class can be used for objects that have solidity, it handles the collision logic between
     *                  the object and Sonic, using this removes duplicated code and ensures that the collision logic
     *                  is consistent across different objects.
     */

    class Solid :
            public Object {

        protected:
            enum Solidity {
                Top = 1,
                LeftRightBottom = 2,
                All = (Top | LeftRightBottom)
            };

        public:
            /**
             * @brief           Constructs a new solid object.
             *
             * @param[in]       subType the subtype.
             * @param[in]       x the X coordinate of the object.
             * @param[in]       y the Y coordinate of the object.
             * @param[in]       rememberState whether the state is remembered when spawned again.
             * @param[in]       mirrored whether the object is mirrored in x.
             * @param[in]       mirrored whether the object is flipped in y.
             */
            Solid(std::string type, int subType, float x, float y, bool rememberState, bool mirrored, bool flipped);

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
             * @copydoc         Nedrysoft::Object::renderCollision(Camera *)
             */
            auto renderCollision(Camera *camera) -> void override;

            /**
             *  @copydoc        Nedrysoft::Object::reset()
             */
            auto reset() -> void override;

        protected:
            auto setCollisionEnabled(bool collisionEnabled) -> void;
            [[nodiscard]] auto isCollisionEnabled() const -> bool;
            auto setStandingOnObject(int distance, TileMap *tileMap, Camera *camera, Sonic *sonic) -> void;

            virtual auto checkForCollisionAction(float xDistance, float yDistance, TileMap *tileMap, Camera *camera,
                                                 class Sonic *sonic) -> bool;
            virtual auto objectUpdating(TileMap *tileMap, Camera *camera, Sonic *sonic) -> void;

            auto createHitBox(AnimationPlayer *animationPlayer, int dx = 0, int dy = 0, int dw = 0, int dh = 0) -> void;

            virtual auto hurtSonic(float yDistance, Sonic *sonic, bool overrideInvulnerability = false) -> void;

        protected:
            AnimationPlayer m_animation;

            SDL_Rect m_collisionRect;

            float m_yPosition;
            float m_ySpeed;

            int m_collisionFlags;

            bool m_collisionEnabled;

            int m_solidity;
    };
}

#endif //NEDRYSOFT_SOLID_H
