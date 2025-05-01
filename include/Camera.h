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

#ifndef NEDRYSOFT_CAMERA_H
#define NEDRYSOFT_CAMERA_H

#include "Structs.h"

#include <SDL2/SDL.h>
#include <list>
#include <vector>

namespace Nedrysoft {
    class Sonic;

    /**
     * @brief       The camera represents the visible (rendered) area of the current play-field.
     *
     * @notes       The viewport is the size of the rendered image, the viewport origin is at 0,0 at the top
     *              left and has a width & height which defines how much of the screen is visible.
     *
     *              The camera tracks the players position relative to the screen and the world, and we
     *              maintain a central area where the players sprite should normally reside in, they may extend
     *              outside this when the player is at any of the extents of the map, since the map stops scrolling
     *              at those extents.
     *
     *              Therefore the player can move inside this central screen area without the camera moving, the
     *              camera moves when the player hits one or more of the sides of this central area.
     */
    class Camera {
        public:
            struct Limit {
                float startX;
                float endX;
                float y;
            };

        public:
            /**
             * @brief       Constructs a the camera object.
             */
            Camera();

            /**
             * @brief       Destroys the camera object.
             */
            ~Camera() = default;

            /**
             * @brief       Updates the camera using the players location.
             *
             * @details     The camera tracks the player using a set of rules that are designed to allow the player
             *              a small amount of freedom of movement before the camera moves, this results in a more
             *              natural feel of the camera than if it "hard tracked" the players position.
             *
             *              We pass in the actual player object rather than a more basic vector as we can make use
             *              of other information that the player has, such is his velocity and origin.
             *
             * @param[in]   player The player instance.
             */
            auto update(class Sonic *player) -> void;

            /**
             * @brief       Sets the limits which determine any restrictions on camera movement.
             *
             * @details     The "ground level" changes depending on where Sonic is in the level.  The map data contains
             *              a list of entries with "start x", "end x" and "y" which determine the physical Y limit
             *              when the player is contained within the start and stop x coordinates.
             *
             *              This is used straight away in the levels to stop the camera panning too low down and
             *              showing empty tiles beneath the ground.
             *
             * @param[in]   limits The limits list.
             */
            auto setLimits(std::list<Limit> limits) -> void;

            /**
             * @brief       Sets the physical limits of the world.
             *
             * @details     Used to set the absolute limits of movement, the player cannot move beyond this.
             *
             * @param[in]   width The width of the tilemap in pixels.
             * @param[in]   height The height of the tilemap in pixels.
             */
            auto setWorldSize(int width, int height) -> void;

            /**
             * @brief       Returns the top left coordinates of the cameras viewport.
             *
             * @returns     The viewports position.
             */
            [[nodiscard]] auto position() const -> Vector;

            /**
             * @brief       Positions the camera at an initial point.
             *
             * @details     Positions the camera ensuring that the viewport is clipped to any physical extents
             *              or limits that are in place for the requested coordinates.  In the case of GHZ, this
             *              means that the initial screen is positioned correctly, if clipping or limits are not
             *              enforced then the camera appears lower than it should resulting in seeing below the
             *              ground level.
             *
             * @param[in]   position The requested position.
             */
            auto setInitialPosition(const Vector &position) -> void;

            /**
             * @brief       Renders an overlay that shows the players movement viewport.
             *
             * @notes       This overlay shows the area which is used to determine when the play-field should
             *              be moved, when the player hits the edges of this area the viewport is scrolled.
             *
             *              This function actually renders the opposite, it renders rectangles outside of this area
             *              in a dark alpha blend, meaning that the viewport is rendered at full brightness, but
             *              the area outside of it appears darker than usual.
             */
            auto renderDebugOverlay() const -> void;

        private:

            /**
             * @brief       Clips coordinates to the world and limit extremes.
             *
             * @details     This function ensures that the camera cannot move outside the world or any limits
             *              imposed by the game.
             *
             * @param[in,out]   positionX the x coordinate to clip.
             * @param[in,out]   positionY the y coordinate to clip.
             */
            auto clipPosition(float *positionX, float *positionY) -> void;

        private:
            float m_worldWidth;
            float m_worldHeight;

            float m_x;
            float m_y;

            float m_verticalFocalPoint;

            std::list<Limit> m_limits;

            std::list<Limit>::iterator m_currentLimit;
    };
}

#endif //NEDRYSOFT_CAMERA_H
