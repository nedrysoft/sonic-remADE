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

#ifndef NEDRYSOFT_OSCILLATOR_H
#define NEDRYSOFT_OSCILLATOR_H

#include <map>
#include <string>
#include <vector>

#include "Sonic.h"

namespace Nedrysoft {
    /**
     * @brief       Maintains a list of synchronized oscillating values.
     *
     * @details     The original game uses these oscillator values to move platforms in the level, as all oscillators
     *              are updated at the same time with constant values they are all in a synchronised state, different
     *              platform sub types will reference a particular oscillator, this means that the behaviour is
     *              predictable and defined, therefore platforms will always be in the right position relative to
     *              one and other.
     *
     *              Each oscillator has it's own midpoint and amplitude, so they are not interchangeable unless you
     *              pick carefully.
     *
     *              The behaviour of the logic works in a similar way to gravity in the rest of the engine, once
     *              the value passes the midpoint we switch from adding the frequency value to the rate to
     *              subtracting (and vice versa the other way), so once it passes the midpoint it slows until
     *              the rate goes negative and the oscillator then starts moving back towards the midpoint.
     */
    class Oscillator {
        public:

            enum OscillatorTypes {
                Normal = 0,
                Fast = 1,
                Alt = 2,
            };

        private:
            enum OscillatorDirection {
                Up,
                Down
            };

            struct OscillatorValue {
                float value;                         //<! the current value of this oscillator
                float rate;                          //<! the current rate of change applied to the value
            };

            struct OscillatorSetting {
                float frequency;                     //<! the accumulator value for the oscillator
                float midPoint;                      //<! the midpoint which the oscillation is around
            };

        public:
            /**
             * @brief       Returns the singleton instance of the oscillator.
             *
             * @returns     the singleton instance.
             */
            static auto getInstance() -> Oscillator *;

            /**
             * @brief       Updates the oscillators.
             *
             * @note        In the original game when sonic is dying/dead then the oscillators stop updating,
             *              and therefore as sonic dies, the platforms will stop moving.
             *
             * @param[in]   sonic the Sonic instance.
             */
            auto update(Sonic *sonic) -> void;

            /**
             * @brief       Returns the current oscillator value.
             *
             * @param[in]   oscillatorId the height of the oscillator to retrieve.
             * @param[in]   type the type of the oscillator to find.
             * @param[out]  midPoint the midpoint of the oscillator.
             *
             * @return
             */
            auto value(int height, OscillatorTypes type = Normal, float *midPoint = nullptr) -> float;

            /**
             * @brief       Reinitialize all the oscillators.
             */
            auto initialise() -> void;

        private:
            /**
             * @brief       Constructs the oscillator.
             */
            Oscillator();

            /**
             * @brief       Registers an oscillator.
             *
             * @param[in]   height the height of the oscillator.
             * @param[in]   type the type of oscillator.
             */
            auto registerOscillator(int height, OscillatorTypes type = OscillatorTypes::Normal) -> void;

        private:
            std::vector<OscillatorDirection> m_directions;  //<! bit mask containing the current direction of each oscillator

            std::vector<OscillatorValue> m_values;          //<! the current oscillator values
            std::vector<OscillatorSetting> m_settings;      //<! the oscillator settings

            std::map<std::string, int> m_types;
    };
}

#endif //NEDRYSOFT_OSCILLATOR_H
