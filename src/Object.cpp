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

#include "Object.h"

#include "Sonic.h"
#include "Utils.h"

#include <utility>

Nedrysoft::Object::Object(std::string type, int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        m_type(std::move(type)),
        m_subType(subType),
        m_x(x),
        m_y(y),
        m_rememberState(rememberState),
        m_mirrored(mirrored),
        m_flipped(flipped),
        m_startingX(x),
        m_startingY(y) {

    N_UNUSED(m_mirrored)
    N_UNUSED(m_subType)
    N_UNUSED(m_flipped)
}

Nedrysoft::Object::~Object() = default;

auto Nedrysoft::Object::type() const -> std::string {
    return m_type;
}

auto Nedrysoft::Object::subType() const -> int {
    return m_subType;
}

auto Nedrysoft::Object::x() const -> float {
    return m_x;
}

auto Nedrysoft::Object::y() const -> float {
    return m_y;
}

auto Nedrysoft::Object::rememberState() const -> bool {
    return m_rememberState;
}

auto Nedrysoft::Object::spawn(Camera *camera) -> bool {
    N_UNUSED(camera)

    return true;
}
auto Nedrysoft::Object::despawn(Camera *camera) -> void {
    N_UNUSED(camera)
}

auto Nedrysoft::Object::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    N_UNUSED(tileMap)
    N_UNUSED(sonic)
    N_UNUSED(camera)

    return true;
}

auto Nedrysoft::Object::render(Camera *camera, ObjectRenderPriority priority) -> void {
    N_UNUSED(camera)
    N_UNUSED(priority)

}

auto Nedrysoft::Object::update() -> void {

}

auto Nedrysoft::Object::renderCollision(Camera *camera) -> void {
    N_UNUSED(camera);
}

auto Nedrysoft::Object::reset() -> void {
    m_x = m_startingX;
    m_y = m_startingY;
}