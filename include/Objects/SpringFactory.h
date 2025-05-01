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

#ifndef NEDRYSOFT_SPRINGFACTORY_H
#define NEDRYSOFT_SPRINGFACTORY_H

#include "Object.h"
#include "ObjectFactory.h"
#include "Spring.h"

#include <string>

namespace Nedrysoft {
    class Animations;

    class SpringFactory :
            public ObjectFactory {

        public:
            static auto getInstance() -> SpringFactory *;

            static auto initialise() -> SpringFactory *;

            /**
             *  @copydoc        Nedrysoft::ObjectFactory::update()
             */
            auto animations() -> Nedrysoft::Animations * override;

            /**
             *  @copydoc        Nedrysoft::ObjectFactory::update()
             */
            auto update() -> void override;

            /**
             *  @copydoc        Nedrysoft::ObjectFactory::create(int, float, float, bool, mirrored, flipped)
             */
            auto create(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) -> Spring * override;

        private:
            static class Animations *m_animations;
    };
}

#endif //NEDRYSOFT_SPRINGFACTORY_H
