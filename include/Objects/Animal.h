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

#ifndef NEDRYSOFT_ANIMAL_H
#define NEDRYSOFT_ANIMAL_H

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Object.h"

#include <random>

namespace Nedrysoft {
    class Camera;
    class Sonic;

    class Animal :
            public Object {

        public:
            enum AnimalType {
                Blackbird,
                Chicken,
                Flicky,
                Pig,
                Rabbit,
                Seal,
                Squirrel,
                Unknown
            };

        private:
            enum class AnimalState {
                Falling,
                Bouncing
            };

            struct AnimalData {
                std::string name;
                float xSpeed;
                float ySpeed;
                AnimalType type;
            };

        public:
            /**
             * @brief           Constructs a new animal object.
             *
             * @param[in]       x the X coordinate of the object.
             * @param[in]       y the Y coordinate of the object.
             */
            Animal(float x, float y);

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

        private:
            static auto toTypeString(AnimalType type) -> std::string;
            static auto toType(const std::string &name) -> AnimalType;

        private:

            static Animations *m_animations;

            AnimationPlayer m_animation;

            float m_xSpeed;
            float m_ySpeed;

            AnimalState m_state;
            AnimalData m_animal;

            static std::random_device m_randomDevice;
            static std::mt19937 m_randomGenerator;
            static std::uniform_int_distribution<int> m_dist;

            static std::map<std::string, std::vector<AnimalData>> m_data;
    };
}

#endif //NEDRYSOFT_ANIMAL_H
