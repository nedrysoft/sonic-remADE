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

#include "FS.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <regex>

auto Nedrysoft::FS::toNative(const std::string &fileName) -> const std::string {
#if defined(TARGET_WINDOWS)
    //auto filePath = fileName;

    auto filePath = std::regex_replace(fileName, std::regex("/"), "\\"); // replace 'def' -> 'klm'

    //filePath = filePath.replace(filePath.begin(), filePath.end(), "/", "\\");
#else
    auto filePath = fileName;
#endif

    return filePath;
}

auto Nedrysoft::FS::SDL(const std::string &fileName, OpenMode mode) -> SDL_RWops * {
    const char *modeString;

    switch(mode) {
        case OpenMode::Read: {
            modeString = "rb";

            break;
        }

        default: {
            assert(false);
        }
    }

    return SDL_RWFromFile(toNative(fileName).c_str(), modeString);
}

auto Nedrysoft::FS::IOS(const std::string &fileName, OpenMode mode) -> std::ifstream {
    std::ifstream file(toNative(fileName));

    if (!file) {
        std::cerr << "failed to open file: " << fileName << std::endl;
    }

    return file;
}

auto Nedrysoft::FS::BLOB(const std::string &fileName, OpenMode mode) -> Magick::Blob {
    auto file = fopen(toNative(fileName).c_str(), "rb");

    assert(file != nullptr);

    auto fileSize = std::filesystem::file_size(fileName);

    auto data = static_cast<char *>(malloc(fileSize));

    auto bytesRead = fread(data, 1, fileSize, file);

    assert(bytesRead == fileSize);

    Magick::Blob blob(data, fileSize);

    fclose(file);

    free(data);

    return blob;
}