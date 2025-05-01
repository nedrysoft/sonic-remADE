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

#ifndef NEDRYSOFT_OBJECT_H
#define NEDRYSOFT_OBJECT_H

#include <string>

namespace Nedrysoft {
    class Camera;
    class Sonic;
    class TileMap;

    enum class ObjectRenderPriority {
        None = 0,
        PriorityOnly = 1,
        NonPriorityOnly = 2,
        All = 4
    };

    /**
     * @brief           The base class for an in game object.
     *
     * @details         This class provides the engine with a way to maintain in game objects, both keeping
     *                  then up to date with respect to the current state of the game, but also doing performing
     *                  rendering of the object.
     *
     * @note            This class is not intended to be instantiated directly, but it may be used as a way of
     *                  creating null objects during development.
     */

    class Object {
        public:
            /**
             * @brief           Constructs a new object.
             *
             * @param[in]       type the name of the object (i.e "rings" or "enemy-crabmeat")
             * @param[in]       subType the sub type information.
             * @param[in]       x the X coordinate of the object.
             * @param[in]       y the Y coordinate of the object.
             * @param[in]       rememberState true if the object should remember its state across re-spawns.
             * @param[in]       mirrored true if the object should be mirrored in X.
             * @param[in]       flipped the if the object should be mirrored in Y.
             */
            Object(std::string type, int subType, float x, float y, bool rememberState, bool mirrored, bool flipped);

            /**
             * @brief           Destroys the object.
             */
            ~Object();

            /**
             * @brief           The class wide update object function.
             *
             * @details         This function is intended to provide an object with a means of updating a
             *                  class wide state.  For example, the rings object maintains a single normal
             *                  animation player, each instance of a ring uses this for rendering, resulting
             *                  in all rings being synchronised.
             *
             *                  This function is called by the object manager once per frame, so it is used
             *                  by the rings object to advance the normal ring animation by one frame.
             */
            static auto update() -> void;

            /**
             * @brief           Returns the type name of the object.
             *
             * @details         The type name is derived directly from the objects JSON "id" entry.
             *
             * @returns         The name of the object.
             */
            [[nodiscard]] auto type() const -> std::string;

            /**
             * @brief           Returns the sub type of the object.
             *
             * @details         Each object instance includes a sub-type field that provides the object
             *                  width additional information, for example for rings it tells the instance
             *                  now many rings there are in this group and how they are arranged.
             *
             * @returns         The subtype.
             */
            [[nodiscard]] auto subType() const -> int;

            /**
             * @brief           Returns the X coordinated of the object.
             *
             * @brief           This coordinate is the "primary" point of the object.  The object may
             *                  extend out from this point, i.e rings may appear either side of it.
             *
             * @returns         The X coordinate.
             */
            [[nodiscard]] auto x() const -> float;

            /**
             * @brief           Returns the Y coordinated of the object.
             *
             * @brief           This coordinate is the "primary" point of the object.  The object may
             *                  extend out from this point, i.e rings may appear above or below of it.
             *
             * @returns         The Y coordinate.
             */
            [[nodiscard]] auto y() const -> float;

            /**
             * @brief           Whether the object remembers its state.
             *
             * @brief           Most interactive objects will remember their state, but some objects do
             *                  not have this need, for example the purple rock in GHZ.
             *
             * @returns         True if the object remembers its state; otherwise false.
             */
            [[nodiscard]] auto rememberState() const -> bool;

            /**
             * @brief           Called to spawn an object.
             *
             * @details         When an object enters the "spawn viewport", this function is called as the
             *                  engine is preparing to spawn the object, it may be the case that the object
             *                  no longer needs to be spawned (i.e the enemy is dead or the ring has already
             *                  been collected).
             *
             *                  This function allows the object to set it's state for respawning, the object
             *                  can return false from this function if the object is no longer active,
             *                  this prevents the needless spawning of dead objects.
             *
             * @param[in]       camera The camera.
             *
             * @returns         True if the object spawning should complete; otherwise false.
             */
            virtual auto spawn(Camera *camera) -> bool;

            /**
             * @brief           Called when a an object is being despawned.
             *
             * @details         This function allows the engine to save any state information if necessary
             *                  that may be required later on if the object is re-spawned.
             *
             *                  It's unlikely that this function needs to be implemented by any object.
             *
             * @param[in]       camera The camera.
             */
            virtual auto despawn(Camera *camera) -> void;

            /**
             * @brief           Updates this instance of the object.
             *
             * @details         In addition to the class providing an update function, the engine also
             *                  calls the update() function on any object that is currently spawned.  This
             *                  allows the object to interact with Sonic.
             *
             *                  In the case of the rings, this function checks whether sonic has hit the
             *                  ring, if he has it then starts to transition the object to it's collected
             *                  state.
             *
             * @note            It is possible to get the engine to immediately despawn an object after
             *                  it has been updated, this would immediately remove it from the spawned
             *                  object list.  It's unlikely that this is ever the case though, as de-spawning
             *                  immediately would stop any rendering that frame, and it's more likely
             *                  that objects will run for a number of frames after they have transitioned
             *                  to a "collected" or "dead" state.
             *
             * @param[in]       tileMap the tilemap.
             * @param[in]       camera the current camera viewport.
             * @param[in]       sonic the sonic object.
             *
             * @returns     True if the object should be despawned; otherwise false.
             */
            virtual auto update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool;

            /**
             * @brief           Renders the object.
             *
             * @details         Called by the object manager to render the object.
             *
             * @param[in]       camera the camera viewport.
             * @param[in]       priority modifies how the object is rendered.
             */
            virtual auto render(Camera *camera, ObjectRenderPriority priority = ObjectRenderPriority::All) -> void;

            /**
             * @brief           Renders the objects collision box(es).
             *
             * @details         Called by the object manager to render the object.
             *
             * @param[in]       camera the camera viewport.
             */
            virtual auto renderCollision(Camera *camera) -> void;

            /**
             * @brief           Resets the objects state including any remembered information.
             */
            virtual auto reset() -> void;

        protected:
            std::string m_type;
            
            int m_subType;

            float m_x;
            float m_y;

            bool m_rememberState;
            bool m_mirrored;
            bool m_flipped;

        private:            
            float m_startingX;
            float m_startingY;
  
    };
}

#endif //NEDRYSOFT_OBJECT_H
