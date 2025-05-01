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

#include "WaterfallSound.h"

#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Object.h"
#include "Sonic.h"

#include <SDL2/SDL.h>

auto constexpr TwirlSpeed = 0x30;

Nedrysoft::WaterfallSound::WaterfallSound(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Object("waterfall-sound", subType, x, y, rememberState, mirrored, flipped) {

    reset();
}

auto Nedrysoft::WaterfallSound::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    if (fabs(sonic->position().x() - m_x) < Nedrysoft::GameRenderer::getInstance()->viewportWidth()) {
        if (!m_playingSound) {
            Nedrysoft::Audio::getInstance()->playSequence(Nedrysoft::SoundId::WaterfallIn, Nedrysoft::SoundId::WaterfallLoop, Nedrysoft::SoundId::WaterfallOut);

            m_playingSound = true;
        }
    } else {
        if (m_playingSound) {
             Nedrysoft::Audio::getInstance()->endSequence();

             m_playingSound = false;
        }
    }

    return false;
}

auto Nedrysoft::WaterfallSound::update() -> void {

}

auto Nedrysoft::WaterfallSound::spawn(Camera *camera) -> bool {
    //Nedrysoft::Audio::getInstance()->playSequence(Nedrysoft::SoundId::WaterfallIn, Nedrysoft::SoundId::WaterfallLoop, Nedrysoft::SoundId::WaterfallOut);

    return true;
}

auto Nedrysoft::WaterfallSound::despawn(Camera *camera) -> void {
    if (m_playingSound) {
        Nedrysoft::Audio::getInstance()->endSequence();
    }
}

auto Nedrysoft::WaterfallSound::reset() -> void {
    m_playingSound = false;
}
