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

#include "Platform.h"

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Input.h"
#include "Object.h"
#include "Oscillator.h"
#include "PlatformFactory.h"
#include "Sonic.h"

#include <SDL2/SDL.h>

auto constexpr NormalSpeed = 0.25f;
auto constexpr SlowSpeed = 0.1875f;
auto constexpr Gravity = 0.21875f;

Nedrysoft::Platform::Platform(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Solid("platform", subType, x, y, rememberState, mirrored, flipped) {

    reset();
}

auto Nedrysoft::Platform::spawn(Camera *camera) -> bool {
    auto animations = Nedrysoft::PlatformFactory::getInstance()->animations();

    assert(animations);

    switch(m_subType) {
        case Large: {
            animations->start("large", &m_animation);
            break;
        }

        default: {
            animations->start("small", &m_animation);
            break;
        }
    }

    createHitBox(&m_animation, 0, 8, 0, -6);

    return true;
}

auto Nedrysoft::Platform::update() -> void {

}
/*
 *
 * v_oscillating_0_to_20:		equ v_oscillating_table
v_oscillating_0_to_30:		equ v_oscillating_table+4
v_oscillating_0_to_40:		equ v_oscillating_table+8
v_oscillating_0_to_60:		equ v_oscillating_table+$C
v_oscillating_0_to_40_fast:	equ v_oscillating_table+$10
v_oscillating_0_to_10:		equ v_oscillating_table+$14
v_oscillating_0_to_80_fast:	equ v_oscillating_table+$18
v_oscillating_0_to_80:		equ v_oscillating_table+$1C
v_oscillating_0_to_A0:		equ v_oscillating_table+$20
v_oscillating_0_to_A0_alt:	equ v_oscillating_table+$24
v_oscillating_0_to_40_alt:	equ v_oscillating_table+$28
v_oscillating_0_to_60_alt:	equ v_oscillating_table+$2C
v_oscillating_0_to_A0_fast:	equ v_oscillating_table+$30
v_oscillating_0_to_E0:		equ v_oscillating_table+$34
 */
auto Nedrysoft::Platform::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    auto viewportHeight = Nedrysoft::GameRenderer::getInstance()->viewportHeight();
    auto oscillator = Nedrysoft::Oscillator::getInstance();

    auto dx = 0.0f;
    auto dy = 0.0f;
    auto midPoint = 0.0f;

    switch(m_subType) {
        case StationaryPrimary: {
            break;
        }

        case RightToLeft: {
            auto nextOscillatorValue = oscillator->value(128, Oscillator::Fast, &midPoint);

            m_x = m_startingX + m_value - midPoint;

            if (sonic->standingOnObject() == this) {
                dx = nextOscillatorValue - m_value;
            }

            m_value = nextOscillatorValue;

            break;
        }

        case DownToUp: {
            auto nextOscillatorValue = oscillator->value(128, Oscillator::Fast, &midPoint);

            m_y = m_startingY + m_value - midPoint;

            if (sonic->standingOnObject() == this) {
                dy = m_value - nextOscillatorValue;
            }

            m_value = nextOscillatorValue;

            break;
        }

        case FallsWhenStoodOn: {
            switch(m_state) {
                case Normal: {
                    if (sonic->standingOnObject() == this) {
                        m_state = WaitingToFall;
                        m_timer = 30;
                    }

                    break;
                }

                case WaitingToFall: {
                    if (m_timer == 0) {
                        m_subType = Falling;
                        m_timer = 30;
                    } else {
                        m_timer--;
                    }

                    break;
                }
            }

            break;
        }

        case Falling: {
            if (m_timer) {
                m_timer--;
                m_ySpeed = Gravity;
            } else {
                m_ySpeed += Gravity;
                m_y += m_ySpeed;
            }

            break;
        }

        case LeftToRight: {
            auto nextOscillatorValue = oscillator->value(128, Oscillator::Fast, &midPoint);

            m_x = m_startingX - m_value + midPoint;

            if (sonic->standingOnObject() == this) {
                dx = m_value - nextOscillatorValue;
            }

            m_value = nextOscillatorValue;

            break;
        }

        case UpToDown: {
            auto nextOscillatorValue = oscillator->value(128, Oscillator::Fast, &midPoint);

            m_y = m_startingY - m_value + midPoint;

            if (sonic->standingOnObject() == this) {
                dy = nextOscillatorValue - m_value;
            }

            m_value = nextOscillatorValue;

            break;
        }

        case DownToUpSlow: {
            auto nextOscillatorValue = oscillator->value(96, Oscillator::Normal, &midPoint);

            m_y = m_startingY + m_value - midPoint;

            if (sonic->standingOnObject() == this) {
                dy = nextOscillatorValue - m_value;
            }

            m_value = nextOscillatorValue;

            break;
        }

        case UpToDownSlow: {
            auto nextOscillatorValue = oscillator->value(96, Oscillator::Normal, &midPoint);

            m_y = m_startingY - m_value + midPoint;

            if (sonic->standingOnObject() == this) {
                dy = m_value - nextOscillatorValue - 1;
            }

            m_value = nextOscillatorValue;

            break;
        }

    /*


                MovesUpWhenSwitchPressed,
                MovesUp,
                StationarySecondary,
                Large,
                DownToUpSlow
                */

    }

    if (sonic->standingOnObject() == this) {
        sonic->move(dx, dy);
    }

    createHitBox(&m_animation, 0, 8, 0, -6);

    return Solid::update(tileMap, camera, sonic);
}

auto Nedrysoft::Platform::reset() -> void {
    Solid::reset();

    m_solidity = Solidity::Top;
    m_startingX = m_x;
    m_startingY = m_y;
    m_value = 0x80;

    if (m_subType == PlatformType::FallsWhenStoodOn) {
        m_timer = 30;
    }

    m_state = Normal;
}  
