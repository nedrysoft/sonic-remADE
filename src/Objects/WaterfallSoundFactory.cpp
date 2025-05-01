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

#include "WaterfallSoundFactory.h"

auto Nedrysoft::WaterfallSoundFactory::getInstance() -> WaterfallSoundFactory * {
    static Nedrysoft::WaterfallSoundFactory instance;

    return &instance;
}

auto Nedrysoft::WaterfallSoundFactory::initialise() -> WaterfallSoundFactory * {
    return getInstance();
}

auto Nedrysoft::WaterfallSoundFactory::update() -> void {
    Nedrysoft::WaterfallSound::update();
}

auto Nedrysoft::WaterfallSoundFactory::create(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) -> WaterfallSound * {
    return new WaterfallSound(subType, x, y, rememberState, mirrored, flipped);
}
