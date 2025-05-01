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

#ifndef NEDRYSOFT_AUDIO_H
#define NEDRYSOFT_AUDIO_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <map>

#define PlaybackChannels        4
#define AudioChannels           2

typedef int16_t audiosample_t;

namespace Nedrysoft {
    enum class MusicId {
        Title,
        GHZ1
    };

    enum class SoundId {
        ActionBlock,
        Basaran,
        Bomb,
        Bonus,
        BossHit,
        Break,
        Bubble,
        Bumper,
        Burning,
        BuzzExplode,
        ChainRise,
        ChainStomp,
        Collapse,
        Continue,
        Dash,
        Death,
        Diamonds,
        Ding,
        Door,
        Drown,
        Electricity,
        EnterSpecialStage,
        FireBall,
        Flame,
        GiantRing,
        Goal,
        Jump,
        Lamppost,
        Push,
        Register,
        RingLoss,
        RingLeft,
        RingRight,
        Roll,
        Rumbling,
        Saw,
        Shield,
        Signpost,
        Skid,
        Smash,
        SpikeHit,
        SpikeMove,
        Splash,
        Spring,
        Switch,
        WaterfallIn,
        WaterfallOut,
        WaterfallLoop,

        MaxSamples
    };

    enum {
        Idle,
        PlayingIntro,
        PlayingLoop,
        Ending,
        PlayingOuttro
    };

    struct SampleInfo {
        SoundId id;
        audiosample_t *sampleStart;
        audiosample_t *sampleEnd;
    };

    struct SamplePlayback {
        SampleInfo *sample;
        audiosample_t *currentSample;
    };

    struct MusicInfo {
        audiosample_t *start;                               //!< THe pointer to the first sample in the music. (the first sample in the intro)

        audiosample_t *introStart;                          //!< The pointer to the start of the music.
        audiosample_t *introEnd;                            //!< The pointer to the end of the music, leads into the loop.
        audiosample_t *loopStart;                           //!< The pointer to the start of the loop.
        audiosample_t *loopEnd;                             //!< The pointer to the end of the loop, music loops back to loop start.

        audiosample_t *end;                                 //!< The pointer to the last sample of music. (the last sample in the loop)
    };

    struct SequenceSampleInfo {
        audiosample_t *start;                               //!< THe pointer to the first "sample" in the sequence. (the first sample in the intro)

        audiosample_t *introStart;                          //!< The pointer to the start of the intro.
        audiosample_t *introEnd;                            //!< The pointer to the end of the intro, leads into the loop.
        audiosample_t *loopStart;                           //!< The pointer to the start of the loop.
        audiosample_t *loopEnd;                             //!< The pointer to the end of the loop, "sample" loops back to loop start.
        audiosample_t *outtroStart;                         //!< The pointer to the start of the outtro.
        audiosample_t *outtroEnd;                           //!< The pointer to the end of the loop.

        audiosample_t *end;                                 //!< The pointer to the last sample of music. (the last sample in the loop)
    };

    struct SequenceSamplePlayback {
        SequenceSampleInfo sequence;
        audiosample_t *currentSample;
        int state;
    };

    struct MusicPlayback {
        MusicInfo *music;
        audiosample_t *currentSample;
    };

    class Audio {
        public:
            static auto getInstance() -> Audio *;

            auto start() const -> void;
            auto stop() const -> void;

            auto stopMusic() -> void;

            auto playMusic(MusicId id) -> void;

            auto playSample(int channel, Nedrysoft::SoundId id, bool replace = false) -> void;

            /**
             * @brief       Starts playing a sequence sample.
             * @param[in]   introId the sample id of the intro.
             * @param[in]   loopId the sample id of the loop.
             * @param[in]   outtroId the sample id of the outtro.
             */
            auto playSequence(Nedrysoft::SoundId introId, Nedrysoft::SoundId loopId, Nedrysoft::SoundId outtroId) -> void;

            /**
             * @brief       Ends the current playing sequence if one is playing.
             *
             * @details     This function when called will end the playback of the sequence, the sequence will finish
             *              current loop sample and begin playing the outtro, once the outtro has finished
             *              playing the sequence is cleared and will no longer be playing.
             */
            auto endSequence() -> void;

            auto initialise() -> void;

            auto disableAudio(bool disabled = true) -> void;

        private:
            Audio();
            ~Audio() = default;

            auto loadSample(Nedrysoft::SoundId id, const std::string &fileName) -> void;
            auto loadMusic(MusicId id, const std::string &introFileName, const std::string &loopFileName = std::string()) -> void;

            static auto transferSamples(SDL_AudioSpec *audioSpec, audiosample_t *outputSamples, void *inputSamples, int totalSamples) -> bool;

            static auto audioCallback(void *userdata, uint8_t *stream, int32_t len) -> void;

        private:

            std::map<MusicId, MusicInfo> m_music;

            MusicPlayback m_playingMusic;
            SequenceSamplePlayback m_playingSequence;

            SamplePlayback m_playingSamples[PlaybackChannels];         //!< Any samples currently being played.

            std::vector<SampleInfo> m_samples;                         //!< The samples available to play.

            SDL_AudioDeviceID m_audioDevice;                           //!< The audio device used for playback.

            bool m_audioDisabled;
    };
}


#endif //NEDRYSOFT_AUDIO_H
