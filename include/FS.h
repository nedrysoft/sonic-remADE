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

#ifndef NEDRYSOFT_FS_H
#define NEDRYSOFT_FS_H

#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <Magick++.h>

namespace Nedrysoft {
    enum class OpenMode {
        Read,
        Write,
        ReadWrite
    };

    class FS {
        public:
            static auto toNative(const std::string &fileName) -> const std::string;

            static auto SDL(const std::string &fileName, OpenMode mode = OpenMode::Read) -> SDL_RWops *;
            static auto IOS(const std::string &fileName, OpenMode mode = OpenMode::Read) -> std::ifstream;
            static auto BLOB(const std::string &fileName, OpenMode mode = OpenMode::Read) -> Magick::Blob;
    };

}

#endif //NEDRYSOFT_FS_H
