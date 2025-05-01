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

#ifndef NEDRYSOFT_UTILS_H
#define NEDRYSOFT_UTILS_H

#include "Color.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

#define N_UNUSED(x) (void) x;

float constexpr degToRad(float degrees) {
    return degrees * static_cast<float>(M_PI) / 180.0f;
}

template< typename T >
constexpr float sign(T value) {
    if (value > 0) {
        return 1;
    } else if (value < 0) {
        return -1;
    }

    return 0;
}

auto hexStringToInt(const std::string &hexString) -> uint32_t;

template< typename T >
std::string toHexString(T i) {
    std::stringstream stream;

    stream << "0x" << std::setfill ('0') << std::setw(sizeof(T)*2) << std::hex << static_cast<uint64_t>(i);

    return stream.str();
}

template< typename T >
std::string toHexString(T i, int width = 0) {
    std::stringstream stream;

    stream << std::hex << static_cast<uint64_t>(i);

    std::string string = stream.str();

    if ((width) && (string.length() < width)) {
        /*std::stringstream paddingStream;

        paddingStream << std::setfill ('0') << std::setw(width - static_cast<int>(string.length()));*/

        /*std::string paddingString;

        for (int i = 0; i < (width - string.length()) + 1; i++) {
            paddingString + "0";
        }

        string = paddingString + string;*/
    }

    return string;
}

template< typename T >
std::string toIntString(T i, int width = 0) {
    std::stringstream stream;

    stream << i;

    std::string string = stream.str();

    if ((width) && (string.length() < width)) {
        std::stringstream paddingStream;

        paddingStream << std::setfill ('0') << std::setw(width - static_cast<int>(string.length()));

        string = paddingStream.str() + string;
    }

    return string;
}

auto debugValue(
    const std::string &key,
    const std::string &value,
    const std::string &keyColour = "Default",
    const std::string &valueColour = "Default",
    const std::string &keyAttributes = "Bold",
    const std::string &valueAttributes = "Default"
) -> std::string;

#endif //NEDRYSOFT_UTILS_H
