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

#include "AnimationPlayer.h"

#include "Animation.h"
#include "AnimationSprite.h"
#include "AnimationStep.h"
#include "Animations.h"

#include <iostream>

Nedrysoft::AnimationPlayer::AnimationPlayer() :
        m_duration(0),
        m_durationFunction(nullptr),
        m_durationOverride(-1),
        m_currentAnimation(-1) {

}

auto Nedrysoft::AnimationPlayer::setDuration(int duration) -> void {
    m_durationOverride = duration;
    m_durationFunction = nullptr;
}

auto Nedrysoft::AnimationPlayer::setDuration(const FrameDurationFunction &duration) -> void {
    m_durationFunction = duration;
    m_durationOverride = -1;

    m_duration = m_durationFunction(m_animations[m_currentAnimation]->name(), this);
}

auto Nedrysoft::AnimationPlayer::setAnimation(Animation *animation) -> void {
    m_animations.clear();
    m_steps.clear();

    m_animations.push_back(animation);
    m_steps.push_back(animation->start());

    m_currentAnimation = 0;
}

auto Nedrysoft::AnimationPlayer::animation() const -> Animation * {
    return m_animations.empty() ? nullptr : m_animations[m_currentAnimation];
}

auto Nedrysoft::AnimationPlayer::next() -> void {
    if (m_duration > -1) {
        m_duration--;

        return;
    }

    for (auto i = 0; i < m_steps.size(); i++) {
        while(true) {
            m_steps[i] = m_steps[i]->next();

            if (m_steps[i]->type() == AnimationStepType::Show) {
                break;
            }
        }
    }

    if (m_durationFunction) {
        m_duration = m_durationFunction(m_animations[m_currentAnimation]->name(), this);
    } else {
        if (m_durationOverride != -1) {
            m_duration = m_durationOverride;
        } else {
            if (m_animations[m_currentAnimation]->speed() == -1) {
                m_duration = 23;
            } else {
                m_duration = m_animations[m_currentAnimation]->speed();
            }
        }
    }
}

auto Nedrysoft::AnimationPlayer::current() -> AnimationStep * {
    return m_animations.empty() ? nullptr : m_steps[m_currentAnimation];
}

auto Nedrysoft::AnimationPlayer::texture(int *width, int *height, Nedrysoft::Vector *origin) -> SDL_Texture * {
    if (m_animations.empty()) {
        return nullptr;
    }

    auto showStep = dynamic_cast<AnimationShowStep *>(m_steps[m_currentAnimation]);

    if (!showStep) {
        return nullptr;
    }

    if (width) {
        *width = showStep->sprite()->width();
    }

    if (height) {
        *height = showStep->sprite()->height();
    }

    if (origin) {
        *origin = showStep->sprite()->origin();
    }

    return showStep->sprite()->texture();
}

auto Nedrysoft::AnimationPlayer::width() -> float {
    if (m_animations.empty()) {
        return 0;
    }

    auto showStep = dynamic_cast<AnimationShowStep *>(m_steps[m_currentAnimation]);

    return (showStep ? static_cast<float>(showStep->sprite()->width()) : 0);
}

auto Nedrysoft::AnimationPlayer::height() -> float {
    if (m_animations.empty()) {
        return 0;
    }

    auto showStep = dynamic_cast<AnimationShowStep *>(m_steps[m_currentAnimation]);

    return (showStep ? static_cast<float>(showStep->sprite()->height()) : 0);
}

auto Nedrysoft::AnimationPlayer::setAnimations(const std::list<Animation *> &animations) -> void {
    m_animations.clear();
    m_steps.clear();

    for (auto animation : animations) {
        m_animations.push_back(animation);
        m_steps.push_back(animation->start());
    }

    m_currentAnimation = 0;
}

auto Nedrysoft::AnimationPlayer::selectAlternative(int alternative) -> void {
    if (alternative < m_animations.size()) {
        m_currentAnimation = alternative;
    }
}

auto Nedrysoft::AnimationPlayer::isNull() -> bool {
    return m_steps.empty();
}
