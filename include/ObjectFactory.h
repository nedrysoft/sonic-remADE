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

#ifndef NEDRYSOFT_OBJECTFACTORY_H
#define NEDRYSOFT_OBJECTFACTORY_H

#include "Object.h"

#include <string>

namespace Nedrysoft {
    class Animations;

    class ObjectFactory {
        public:
            static auto getInstance() -> ObjectFactory *;

            static auto initialise() -> ObjectFactory *;

            virtual auto animations() -> Nedrysoft::Animations *;

            virtual auto update() -> void;
            virtual auto create(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) -> Object *;
    };
}

#endif //NEDRYSOFT_OBJECTFACTORY_H
