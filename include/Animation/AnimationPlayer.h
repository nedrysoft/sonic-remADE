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

#ifndef NEDRYSOFT_ANIMATIONPLAYER_H
#define NEDRYSOFT_ANIMATIONPLAYER_H

#include "Structs.h"

#include <SDL2/SDL.h>
#include <functional>
#include <string>
#include <list>
#include <vector>

namespace Nedrysoft {
    class Animation;
    class AnimationStep;
    class AnimationPlayer;

    typedef const std::function<int(std::string animationName, class Nedrysoft::AnimationPlayer *animationPlayer)> FrameDurationFunction;

    /**
     * @brief           The AnimationPlayer class provides an object that will is used to run an animation script for
     *                  a sprite.  It will run through each sprite at the configured rate and handles the sprite
     *                  chaining and positioning that Sonic uses.
     */

    class AnimationPlayer {
        public:
            /**
             * @brief           Constructs a new animation player class.
             */
            AnimationPlayer();

            /**
             * @brief           Overrides the default duration that a frame is displayed for, using a function
             *                  that the user provides.
             *
             * @param[in]       duration the frame duration.
             */
            auto setDuration(int duration) -> void;

            /**
             * @brief           Overrides the default duration that a frame is displayed for, using a function
             *                  that the user provides.
             *
             * @param[in]       duration the function to update the duration.
             */
            auto setDuration(const FrameDurationFunction &duration) -> void;

            /**
             * @brief           Sets the animation that the instance will run.
             *
             * @param[in]       animation the animation.
             */
            auto setAnimation(Animation *animation) -> void;

            /**
             * @brief           Sets the groups of animations that the instance will run.
             *
             * @details         Some animations have alternative animations, such as walking or running where there
             *                  are the same animation but at different angles to account for the shape of the terrain,
             *                  we provide this method which will advance each animation so that the correct frame
             *                  can be returned depending on the players ground angle.
             *
             * @param[in]       animation the animation.
             */
            auto setAnimations(const std::list<Animation *> &animations) -> void;

            auto selectAlternative(int alternative) -> void;

            /**
             * @brief           Advances the state of the animation.
             *
             *                  This function should to be called once per frame, the AnimationPlayer will handle
             *                  the changes to the next frame, advancing to a different animation or jumping back to
             *                  repeat the animation from a arbitrary point.
             */
            auto next() -> void;

            /**
             * @brief           Returns the current animation "Step", this is the point in the animation script that
             *                  is currently being shown.
             *
             * @return          A pointer to the current step.
             */
            auto current() -> AnimationStep *;

            /**
             * @brief           Returns the texture for the current sprite that should be displayed.
             *
             * @param[out]      width a pointer which will receive the width of the animation frame or nullptr.
             * @param[out]      height a pointer which will receive the width of the animation frame or nullptr.
             * @param[out]      origin a pointer to a vector that contains the origin point in the sprite or nullptr.
             * 
             * @returns         The SDL texture.
             */
            auto texture(int *width, int *height, Nedrysoft::Vector *origin = nullptr) -> SDL_Texture *;

            /**
             * @brief       Returns the current animation being played.
             *
             * @returns     The animation.
             */
            auto animation() const -> Animation *;

            /**
             * @brief       Returns width of the current frame being displayed.
             *
             * @returns     The width.
             */
            auto width() -> float;

            /**
             * @brief       Returns height of the current frame being displayed.
             *
             * @returns     The width.
             */
            auto height() -> float;

            /**
             * @brief       Returns whether the animation is valid.
             *
             * @returns     True if animation is valid; otherwise false.
             */
            auto isNull() -> bool;

        private:
            int m_duration;
            int m_durationOverride;

            std::function<int(std::string, class Nedrysoft::AnimationPlayer *)> m_durationFunction;

            int m_currentAnimation;

            std::vector<Animation *> m_animations;
            std::vector<AnimationStep *> m_steps;

    };
}

#endif //NEDRYSOFT_ANIMATIONPLAYER_H
