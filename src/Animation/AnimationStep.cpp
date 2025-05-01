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

#include "AnimationStep.h"

#include "Animation.h"
#include "Animations.h"
#include "Utils.h"

Nedrysoft::AnimationStep::AnimationStep() :
    m_type(AnimationStepType::Null),
    m_animation(nullptr),
    m_nextStep(nullptr) {

}

auto Nedrysoft::AnimationStep::type() -> AnimationStepType {
    return m_type;
}

auto Nedrysoft::AnimationStep::animation() -> Animation * {
    return m_animation;
}

auto Nedrysoft::AnimationStep::next() -> AnimationStep * {
    return m_nextStep;
}

auto Nedrysoft::AnimationStep::name() const -> std::string {
    return {};
}

Nedrysoft::AnimationShowStep::AnimationShowStep(std::string id, Animation *parent) :
        AnimationStep(),
        m_name(std::move(id)),
        m_sprite(nullptr) {

    m_type = AnimationStepType::Show;
    m_animation = parent;
}

auto Nedrysoft::AnimationShowStep::update(Animations *animations) -> void {
    m_sprite = animations->sprite(m_name);

    if (!m_sprite) {
        std::cout << color::rize("[animation sprite missing]", "Red", "Default", "Bold") +
                                 debugValue("step", m_name, "Cyan", "Green", "Bold") <<
                                 std::endl;
    }

    for(int itemIndex = 0; itemIndex < m_animation->script().size(); itemIndex++) {
        if (m_animation->script()[itemIndex] == this) {
            if (itemIndex < m_animation->script().size() - 1) {
                m_nextStep = m_animation->script()[itemIndex + 1];
            } else {
                m_nextStep = m_animation->script()[0];
            }

            return;
        }
    }
}

auto Nedrysoft::AnimationShowStep::sprite() -> class AnimationSprite * {
    return m_sprite;
}

auto Nedrysoft::AnimationShowStep::name() const -> std::string {
    return "[animation show](" + m_name + ")";
}

Nedrysoft::AnimationNextStep::AnimationNextStep(int delta, Animation *parent) :
        AnimationStep(),
        m_delta(delta) {

    m_type = AnimationStepType::Next;
    m_animation = parent;
}

auto Nedrysoft::AnimationNextStep::update(Animations *animations) -> void  {
    if (m_animation) {
        m_nextStep = m_animation->script()[(m_animation->script().size() + m_delta) - 1];
    } else {
        m_nextStep = nullptr;
    }
}

Nedrysoft::AnimationChangeStep::AnimationChangeStep(std::string id, Animation *parent) :
        AnimationStep(),
        m_name(std::move(id)),
        m_changeAnimation(nullptr) {

    m_type = AnimationStepType::Change;
    m_animation = parent;
}

auto Nedrysoft::AnimationChangeStep::update(Animations *animations) -> void  {
    auto changeAnimation = animations->find(std::string(m_name));

    if (changeAnimation) {
        m_changeAnimation  = changeAnimation;
        m_nextStep = m_changeAnimation->start();
    } else {
        std::cout << color::rize("[unable to find linked animation]", "Red", "Default", "Bold") +
                         debugValue("from", m_animation->name(), "Cyan", "Green", "Bold") <<
                         debugValue("to", m_name, "Cyan", "Green", "Bold") <<
                         std::endl;
    }
}

auto Nedrysoft::AnimationChangeStep::name() const -> std::string {
    return "[animation change](" + m_changeAnimation->name() + ")";
}
