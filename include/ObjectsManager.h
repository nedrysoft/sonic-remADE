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

#ifndef NEDRYSOFT_OBJECTSMANAGER_H
#define NEDRYSOFT_OBJECTSMANAGER_H

#include "Object.h"
#include "ObjectFactory.h"

#include <SDL2/SDL.h>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace Nedrysoft {
    class Camera;
    class Sonic;

    /**
     * @brief       The objects manager is manages any objects inside the system that are not directly managed
     *              by the sonic object.
     *
     * @details     Some objects in the same are managed by the sonic object as they may
     *              have a direct influence on the sonic, other objects represent objects inside the level that
     *              sonic may interact with, but sonic does not need to know about the object specifically.
     *
     *              The object manager is responsible for spawning (or respawning) objects that are not directly
     *              managed by sonic.
     *
     *              There are other objects in the game that are managed by themselves and not by this object or
     *              by the sonic object.
     */
    class ObjectsManager {
        public:
            /**
             * @brief       Returns the singleton instance of the object manager.
             */
            static auto getInstance() -> ObjectsManager *;

            /**
             * @brief       Loads the object map for the level and returns an instance to manage it.
             *
             * @param[in]   filename The filename of the object to load.
             *
             * @returns     A new instance that manages the given level data.
             */
            auto load(const std::string &filename) -> bool;

            /**
             * @brief       Renders the currently spawned objects.
             *
             * @param[in]   camera The camera.
             * @param[in]   renderPriority Which objects should be rendered.
             */
            auto render(Camera *camera, ObjectRenderPriority renderPriority = ObjectRenderPriority::All) -> void;

            /**
             * @brief       Renders the collision data for the objects.
             *
             * @param[in]   camera The camera.
             */
            auto renderCollisions(Camera *camera) -> void;

            /**
             * @brief       Updates the object manager for the current frame.
             *
             * @details     This function is called each frame and uses the camera object passed in to track
             *              any spawnable objects that may have entered the cameras viewport.
             *
             * @param[in]   tileMap The tile map.
             * @param[in]   camera The camera object.
             * @param[in]   sonic The sonic player object.
             */
            auto update(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> void;

            /**
             * @brief       Returns the total number of spawnable objects visible in the viewport.
             *
             * @param[in]   camera The camera
             *
             * @returns     The total number of visible spawnable objects.
             */
            auto visibleSpawnable(Camera *camera) -> int;

            /**
             * @brief       Adds a dynamic object to the manager.
             *
             * @details     Dynamic objects are created during the game and have a limited life cycle, the major
             *              use is to create the rings that are emitted when sonic is hit.  The manager will delete
             *              these objects once the object indicates that they can be destroyed.
             *
             * @param[in]   object The dynamic object.
             */
            auto addDynamicObject(Object *object) -> void;

            /**
             * @brief      Resets all objects to initial state and removes any dynamic objects.
             */
            auto reset() -> void;

        private:
            /**
             * @brief       Spawns objects.
             * 
             * @details     Objects are spawned and when they are within a specific distance from sonic, this allows
             *              the game to only compute object movements and collisions with objects that can potentially interact
             *              with sonic.
             * 
             * @param[in]   camera the current camera position.
             * @param[in]   sonic the sonic object.
             */
            auto spawnObjects(Camera *camera, class Sonic *sonic) -> void;

            /**
             * @brief       Spawns objects.
             * 
             * @details     Objects are despawned when they leave the specific distance from sonic, this allows
             *              the game to only compute object movements and collisions with objects that can potentially interact
             *              with sonic.
             * 
             * @param[in]   camera the current camera position.
             * @param[in]   sonic the sonic object.
             */
            auto despawnObjects(Camera *camera, class Sonic *sonic) -> void;

            /**
             * @brief       Updates the m_firstObject to point at the first spawnable object that is inside the
             *              current world "column" defined by the viewport.
             *
             * @details     The objects in the manager are sorted by their X coordinate, update() should be called
             *              each frame after sonic has moved.  This function maintains a list iterator that is
             *              always positioned at the first visible item in X governed by the cameras viewport.
             *
             *              The sonic levels are not just expansive horizontally, they also extend vertically, so
             *              the first object pointer covers the entire vertical column.  However, because of the sorting
             *              it's incredibly quick to figure out exactly what objects are spawnable in the viewport because
             *              the m_firstObject represents the first visible object in X and we can simply then iterate
             *              from that point ignoring any objects that have a Y outside of the viewports starting Y and
             *              ending Y, we continue to iterate until we hit an object who's X is outside of the current
             *              column.
             *
             *              At that point we will have considered every object that could be spawnable on screen.
             *
             *              This is very efficient because nearly every object that is outside of the viewport will
             *              not even be considered.
             *
             * @param[in]   camera The camera object.
             */
            auto updateFirstObject(Camera *camera) -> void;

        private:
            std::vector<Object *> m_objects;                        //<! The list of objects loaded into this manager.
            std::list<Object *> m_dynamicObjects;                   //<! The list of objects loaded into this manager.

            std::vector<Object *>::iterator m_firstObject;          //<! The first spawnable object in the viewport.

            std::set<Object *> m_spawnedObjects;

            std::map<std::string, ObjectFactory *> m_factories;

    };
}

#endif //NEDRYSOFT_OBJECTSMANAGER_H
