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

#ifndef NEDRYSOFT_ANIMATIONSTEP_H
#define NEDRYSOFT_ANIMATIONSTEP_H

#include <string>

namespace Nedrysoft {
    class Animation;
    class Animations;
    class AnimationSprite;

    enum class AnimationStepType {
        Null,
        Show,
        Next,
        Change
    };

    /**
     * @brief       The animation step class is a base for each entry that can be used in an animation step, each
     *              step in the script provides an "instruction" that defines how the animation is displayed, for
     *              example a step might show a specific image or jump from one point to another inside the same
     *              animation or change animaton completely.
     */

    class AnimationStep {
        public:
            AnimationStep();
            virtual ~AnimationStep() = default;

            auto type() -> AnimationStepType;

            auto animation() -> Animation *;

            auto next() -> AnimationStep *;

            virtual auto update(Animations *animations) -> void = 0;

            [[nodiscard]] virtual auto name() const -> std::string;

        protected:
            AnimationStepType m_type;

            Animation *m_animation;

            AnimationStep *m_nextStep;
    };

    class AnimationShowStep :
            public AnimationStep {

        public:
            AnimationShowStep(std::string id, Animation *parent);
            ~AnimationShowStep() override = default;

            auto sprite() -> AnimationSprite *;

            auto update(Animations *animations) -> void override;

            [[nodiscard]] auto name() const -> std::string override;

        private:
            struct AnimationSprite *m_sprite;

            std::string m_name;
    };

    class AnimationNextStep :
            public AnimationStep {

        public:
            AnimationNextStep(int delta, Animation *parent);
            ~AnimationNextStep() override = default;

            auto update(Animations *animations) -> void override;

        private:
            int m_delta;
    };

    class AnimationChangeStep :
            public AnimationStep {

        public:
            AnimationChangeStep(std::string id, Animation *parent);
            ~AnimationChangeStep() override = default;

            auto update(Animations *animations) -> void override;

            [[nodiscard]] auto name() const -> std::string override;

        private:
            std::string m_name;

            Animation *m_changeAnimation;
    };
}

#endif //NEDRYSOFT_ANIMATIONSTEP_H
