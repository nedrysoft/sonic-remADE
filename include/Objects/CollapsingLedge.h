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

#ifndef NEDRYSOFT_COLLAPSINGLEDGE_H
#define NEDRYSOFT_COLLAPSINGLEDGE_H

#include "Image.h"
#include "Object.h"

#include <SDL2/SDL.h>
#include <vector>
#include <map>

namespace Nedrysoft {
    class Camera;
    class Sonic;

    class CollapsingLedge :
            public Object {

        private:
            enum CollisionFlags {
                None,
                Top,
                Bottom,
                Left,
                Right
            };

            enum CollisionStates {
                Disabled = 1,
                FirstCollision = 2
            };

            struct Fragment {
                int x;
                float y;
                float ySpeed;
                int sourceX;
                int sourceY;
                int width;
                int height;
                int timing;
                int active;
                bool surface;
            };

            struct Section {
                bool flipped;
                int height;
                int left;
                bool mirrored;
                int paletteLine;
                bool priority;
                int tileIndex;
                int top;
                int width;
            };

            enum class State {
                Normal,
                Waiting,
                Collapsing,
                Collapsed
            };

        public:
            /**
             * @brief           Constructs a new collapsing bridge object.
             *
             * @param[in]       subType the subtype.
             * @param[in]       x the X coordinate of the object.
             * @param[in]       y the Y coordinate of the object.
             * @param[in]       rememberState whether the state is remembered when spawned again.
             * @param[in]       mirrored whether the object is mirrored in x.
             * @param[in]       mirrored whether the object is flipped in y.
             */
            CollapsingLedge(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped);

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
             *  @copydoc        Nedrysoft::Object::reset()
             */
            auto reset() -> void override; 

        private:
            auto setStandingOnObject(int distance, TileMap *tileMap, Camera *camera, Sonic *sonic) -> void;

        private:
            static Nedrysoft::Image *m_image;
            static std::vector<int> m_heights;
            static std::vector<Section> m_sections;
            static std::vector<int> m_timings;

            SDL_Rect m_collisionRect;

            State m_state;
            //int m_collisionFlags;

            static int m_hitboxHeightRadius;
            static int m_hitboxWidthRadius;

            static int m_originX;
            static int m_originY;

            static int m_blockWidth;
            static int m_blockHeight;

            std::vector<Fragment> m_fragments;
            std::map<int, float> m_surfaceOffsets;

            int m_collisionState;
            int m_waitTimer;
    };
}

#endif //NEDRYSOFT_COLLAPSINGLEDGE_H
