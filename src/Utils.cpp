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

#include "Color.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

auto hexStringToInt(const std::string &hexString) -> uint32_t {
    std::stringstream ss;

    ss << std::hex << hexString;

    uint32_t value;

    ss >> value;

    return value;
}

auto debugValue(const std::string &key, const std::string &value, const std::string &keyColour, const std::string &valueColour, const std::string &keyAttributes, const std::string &valueAttributes) -> std::string {
    return " [" + color::rize(key, keyColour, "Default", keyAttributes) + " = " + color::rize(value, valueColour, "Default", valueAttributes) + "]";
}
