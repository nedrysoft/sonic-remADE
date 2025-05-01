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

#include "Audio.h"

#include "FS.h"
#include "Utils.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <map>

Nedrysoft::Audio::Audio() :
        m_playingSamples{},
        m_audioDevice(0),
        m_playingSequence{},
        m_audioDisabled(false) {

    m_playingSequence.currentSample = nullptr;
    m_playingSequence.state = Nedrysoft::Idle;
}

auto Nedrysoft::Audio::getInstance() -> Audio * {
    static Audio instance;

    return &instance;
}

auto Nedrysoft::Audio::loadMusic(MusicId id, const std::string &introFileName, const std::string &loopFileName) -> void {
    SDL_AudioSpec introAudioSpec;
    SDL_AudioSpec loopAudioSpec;

    uint32_t introBufferLength;
    uint8_t *introBuffer;

    if (SDL_LoadWAV_RW(Nedrysoft::FS::SDL(introFileName), true, &introAudioSpec, &introBuffer, &introBufferLength) == nullptr) {
        std::cout << "unable to load audio sample " << introFileName << std::endl;
    }

    uint32_t loopBufferLength = 0;
    uint8_t *loopBuffer = nullptr;

    if (!loopFileName.empty()) {
        if (SDL_LoadWAV_RW(Nedrysoft::FS::SDL(loopFileName), true, &loopAudioSpec, &loopBuffer, &loopBufferLength) == nullptr) {
            std::cout << "unable to load audio sample " << loopFileName << std::endl;
        }
    }

    auto introTotalSamples = 0;
    auto loopTotalSamples = 0;

    if (introBuffer) {
        introTotalSamples = static_cast<int>(introBufferLength / (SDL_AUDIO_BITSIZE(introAudioSpec.format) / 8));
    }

    if (loopBuffer) {
        loopTotalSamples = static_cast<int>(loopBufferLength / (SDL_AUDIO_BITSIZE(loopAudioSpec.format) / 8));
    }

    auto totalSamples = introTotalSamples + loopTotalSamples;

    MusicInfo musicInfo;

    musicInfo.start = reinterpret_cast<audiosample_t *>(malloc(totalSamples * sizeof(audiosample_t)));
    musicInfo.end = musicInfo.start + totalSamples;

    musicInfo.introStart = musicInfo.start;
    musicInfo.introEnd = musicInfo.introStart + introTotalSamples;

    musicInfo.loopStart = (loopBuffer) ? musicInfo.introEnd : nullptr;
    musicInfo.loopEnd = (loopBuffer) ? musicInfo.loopStart + loopTotalSamples : nullptr;

    if (musicInfo.introStart) {
        transferSamples(&introAudioSpec, musicInfo.introStart, introBuffer, introTotalSamples);

        SDL_FreeWAV(introBuffer);
    }

    if (musicInfo.loopStart) {
        transferSamples(&loopAudioSpec, musicInfo.loopStart, loopBuffer, loopTotalSamples);

        SDL_FreeWAV(loopBuffer);
    }

    m_music[id] = musicInfo;

    m_playingMusic.music = nullptr;
    m_playingMusic.currentSample = nullptr;
}

auto Nedrysoft::Audio::initialise() -> void {
    SDL_AudioSpec desiredAudio;
    SDL_AudioSpec obtainedAudio;

    memset(&obtainedAudio, 0, sizeof(SDL_AudioSpec));
    memset(&desiredAudio, 0, sizeof(SDL_AudioSpec));

    desiredAudio.freq = 44100;
    desiredAudio.format = AUDIO_S16;
    desiredAudio.channels = 2;
    desiredAudio.samples = 128;
    desiredAudio.callback = Nedrysoft::Audio::audioCallback;
    desiredAudio.userdata = reinterpret_cast<void *>(this);

    m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredAudio, &obtainedAudio, 0);

    if (!m_audioDevice) {
        std::cout << "unable to open audio device. " << SDL_GetError() << std::endl;
    }

    m_samples.resize(static_cast<int>(Nedrysoft::SoundId::MaxSamples) + 1);

    loadSample(Nedrysoft::SoundId::ActionBlock, "./data/sound/sfx/ActionBlock.wav");
    loadSample(Nedrysoft::SoundId::Basaran, "./data/sound/sfx/Basaran.wav");
    loadSample(Nedrysoft::SoundId::Bomb, "./data/sound/sfx/Bomb.wav");
    loadSample(Nedrysoft::SoundId::Bonus, "./data/sound/sfx/Bonus.wav");
    loadSample(Nedrysoft::SoundId::BossHit, "./data/sound/sfx/BossHit.wav");
    loadSample(Nedrysoft::SoundId::Break, "./data/sound/sfx/Break.wav");
    loadSample(Nedrysoft::SoundId::Bubble, "./data/sound/sfx/Bubble.wav");
    loadSample(Nedrysoft::SoundId::Bumper, "./data/sound/sfx/Bumper.wav");
    loadSample(Nedrysoft::SoundId::Burning, "./data/sound/sfx/Burning.wav");
    loadSample(Nedrysoft::SoundId::BuzzExplode, "./data/sound/sfx/BuzzExplode.wav");
    loadSample(Nedrysoft::SoundId::ChainRise, "./data/sound/sfx/ChainRise.wav");
    loadSample(Nedrysoft::SoundId::ChainStomp, "./data/sound/sfx/ChainStomp.wav");
    loadSample(Nedrysoft::SoundId::Collapse, "./data/sound/sfx/Collapse.wav");
    loadSample(Nedrysoft::SoundId::Continue, "./data/sound/sfx/Continue.wav");
    loadSample(Nedrysoft::SoundId::Dash, "./data/sound/sfx/Dash.wav");
    loadSample(Nedrysoft::SoundId::Death, "./data/sound/sfx/Death.wav");
    loadSample(Nedrysoft::SoundId::Diamonds, "./data/sound/sfx/Diamonds.wav");
    loadSample(Nedrysoft::SoundId::Ding, "./data/sound/sfx/Ding.wav");
    loadSample(Nedrysoft::SoundId::Door, "./data/sound/sfx/Door.wav");
    loadSample(Nedrysoft::SoundId::Drown, "./data/sound/sfx/Drown.wav");
    loadSample(Nedrysoft::SoundId::Electricity, "./data/sound/sfx/Electricity.wav");
    loadSample(Nedrysoft::SoundId::EnterSpecialStage, "./data/sound/sfx/EnterSS.wav");
    loadSample(Nedrysoft::SoundId::FireBall, "./data/sound/sfx/FireBall.wav");
    loadSample(Nedrysoft::SoundId::Flame, "./data/sound/sfx/Flame.wav");
    loadSample(Nedrysoft::SoundId::GiantRing, "./data/sound/sfx/GiantRing.wav");
    loadSample(Nedrysoft::SoundId::Goal, "./data/sound/sfx/Goal.wav");
    loadSample(Nedrysoft::SoundId::Jump, "./data/sound/sfx/Jump.wav");
    loadSample(Nedrysoft::SoundId::Lamppost, "./data/sound/sfx/Lamppost.wav");
    loadSample(Nedrysoft::SoundId::Push, "./data/sound/sfx/Push.wav");
    loadSample(Nedrysoft::SoundId::Register, "./data/sound/sfx/Register.wav");
    loadSample(Nedrysoft::SoundId::RingLeft, "./data/sound/sfx/RingLeft.wav");
    loadSample(Nedrysoft::SoundId::RingRight, "./data/sound/sfx/RingLeft.wav");
    loadSample(Nedrysoft::SoundId::RingLoss, "./data/sound/sfx/RingLoss.wav");
    loadSample(Nedrysoft::SoundId::Roll, "./data/sound/sfx/Roll.wav");
    loadSample(Nedrysoft::SoundId::Rumbling, "./data/sound/sfx/Rumbling.wav");
    loadSample(Nedrysoft::SoundId::Saw, "./data/sound/sfx/Saw.wav");
    loadSample(Nedrysoft::SoundId::Shield, "./data/sound/sfx/Shield.wav");
    loadSample(Nedrysoft::SoundId::Signpost, "./data/sound/sfx/Signpost.wav");
    loadSample(Nedrysoft::SoundId::Skid, "./data/sound/sfx/Skid.wav");
    loadSample(Nedrysoft::SoundId::Smash, "./data/sound/sfx/Smash.wav");
    loadSample(Nedrysoft::SoundId::SpikeHit, "./data/sound/sfx/SpikeHit.wav");
    loadSample(Nedrysoft::SoundId::SpikeMove, "./data/sound/sfx/SpikeMove.wav");
    loadSample(Nedrysoft::SoundId::Splash, "./data/sound/sfx/Splash.wav");
    loadSample(Nedrysoft::SoundId::Spring, "./data/sound/sfx/Spring.wav");
    loadSample(Nedrysoft::SoundId::Switch, "./data/sound/sfx/Switch.wav");
    loadSample(Nedrysoft::SoundId::WaterfallIn, "./data/sound/sfx/Waterfall-In.wav");
    loadSample(Nedrysoft::SoundId::WaterfallOut, "./data/sound/sfx/Waterfall-Out.wav");
    loadSample(Nedrysoft::SoundId::WaterfallLoop, "./data/sound/sfx/Waterfall-Loop.wav");

    loadMusic(Nedrysoft::MusicId::GHZ1, "./data/sound/music/ghz1-intro.wav", "./data/sound/music/ghz1-loop.wav");
    loadMusic(Nedrysoft::MusicId::Title, "./data/sound/music/title.wav");
}

auto Nedrysoft::Audio::start() const -> void {
    SDL_PauseAudioDevice(m_audioDevice, false);
}

auto Nedrysoft::Audio::stop() const -> void {
    SDL_PauseAudioDevice(m_audioDevice, true);
}

auto Nedrysoft::Audio::stopMusic() -> void {
    m_playingMusic.currentSample = nullptr;
}

auto Nedrysoft::Audio::playMusic(MusicId id) -> void {
    m_playingMusic.music = &m_music[id];
    m_playingMusic.currentSample = m_music[id].introStart;
}

auto Nedrysoft::Audio::loadSample(Nedrysoft::SoundId id, const std::string &fileName) -> void {
    SDL_AudioSpec sampleSpec;

    uint32_t sampleLength;
    uint8_t *sampleBuffer;

    if (SDL_LoadWAV_RW(Nedrysoft::FS::SDL(fileName), true, &sampleSpec, &sampleBuffer, &sampleLength) == nullptr) {
        std::cout << "unable to load audio sample " << fileName << std::endl;
    }

    auto totalSamples = static_cast<int>(sampleLength / (SDL_AUDIO_BITSIZE(sampleSpec.format) / 8));

    int sampleId = static_cast<int>(id);

    m_samples[sampleId].sampleStart = reinterpret_cast<audiosample_t *>(malloc(totalSamples * sizeof(audiosample_t) * 2));
    m_samples[sampleId].sampleEnd = m_samples[sampleId].sampleStart + totalSamples;
    m_samples[sampleId].id = id;

    transferSamples(&sampleSpec,  m_samples[sampleId].sampleStart, sampleBuffer, totalSamples);

    SDL_FreeWAV(sampleBuffer);
}

auto Nedrysoft::Audio::playSample(int channel, Nedrysoft::SoundId id, bool replace) -> void {
    if (m_playingSamples[channel].sample) {
        if ((!replace) && (m_playingSamples[channel].sample->id == id)) {
            return;
        }
    }

    m_playingSamples[channel].sample = &m_samples[static_cast<int>(id)];
    m_playingSamples[channel].currentSample = m_samples[static_cast<int>(id)].sampleStart;
}

auto Nedrysoft::Audio::audioCallback(void *userdata, uint8_t *stream, int32_t len) -> void {
    auto audioInstance = reinterpret_cast<Nedrysoft::Audio *>(userdata);

    auto outputSamples = reinterpret_cast<audiosample_t *>(stream);

    int totalSamples = len / static_cast<int>(sizeof(audiosample_t));

    do {
        float mixedSample = 0;

        if (!audioInstance->m_audioDisabled) {
            if (audioInstance->m_playingSequence.currentSample) {
                switch(audioInstance->m_playingSequence.state) {
                    case Nedrysoft::PlayingIntro: {
                        if (audioInstance->m_playingSequence.currentSample == audioInstance->m_playingSequence.sequence.introEnd) {
                            audioInstance->m_playingSequence.state = Nedrysoft::PlayingLoop;
                            audioInstance->m_playingSequence.currentSample = audioInstance->m_playingSequence.sequence.loopStart;
                        }

                        break;
                    }

                    case Nedrysoft::PlayingLoop: {
                        if (audioInstance->m_playingSequence.currentSample == audioInstance->m_playingSequence.sequence.loopEnd) {
                            audioInstance->m_playingSequence.currentSample = audioInstance->m_playingSequence.sequence.loopStart;
                        }

                        break;
                    }

                    case Nedrysoft::PlayingOuttro: {
                        if (audioInstance->m_playingSequence.currentSample == audioInstance->m_playingSequence.sequence.outtroEnd) {
                            audioInstance->m_playingSequence.state = Nedrysoft::Idle;
                            audioInstance->m_playingSequence.currentSample = nullptr;
                        }

                        break;
                    }

                    case Nedrysoft::Ending: {
                        audiosample_t *endingSample = nullptr;

                         if ( (audioInstance->m_playingSequence.currentSample >= audioInstance->m_playingSequence.sequence.introStart) &&
                             (audioInstance->m_playingSequence.currentSample <= audioInstance->m_playingSequence.sequence.introEnd) ) {

                            endingSample = audioInstance->m_playingSequence.sequence.introEnd;
                        } else if ( (audioInstance->m_playingSequence.currentSample >= audioInstance->m_playingSequence.sequence.loopStart) &&
                             (audioInstance->m_playingSequence.currentSample <= audioInstance->m_playingSequence.sequence.loopEnd) ) {

                            endingSample = audioInstance->m_playingSequence.sequence.loopEnd;
                        } else {
                            /**
                             * ruh roh!  If we've got here, bad things have happened....
                             */
                        }

                        if (endingSample) {
                            if (audioInstance->m_playingSequence.currentSample == endingSample) {
                                audioInstance->m_playingSequence.currentSample = audioInstance->m_playingSequence.sequence.outtroStart;
                                audioInstance->m_playingSequence.state = Nedrysoft::PlayingOuttro;
                            }
                        }
                    }
                }

                if (audioInstance->m_playingSequence.currentSample) {
                    mixedSample += (static_cast<float>(*(audioInstance->m_playingSequence.currentSample++)) / 16384.0f) * 0.5f;
                }
            }

            if (audioInstance->m_playingMusic.currentSample) {
                if (audioInstance->m_playingMusic.currentSample == audioInstance->m_playingMusic.music->end) {
                    audioInstance->m_playingMusic.currentSample = audioInstance->m_playingMusic.music->loopStart;
                }

                if (audioInstance->m_playingMusic.currentSample) {
                    mixedSample += (static_cast<float>(*(audioInstance->m_playingMusic.currentSample++)) / 16384.0f) * 0.5f;
                }
            }

            for (auto &m_playingSample : audioInstance->m_playingSamples) {
                if (m_playingSample.currentSample) {
                    mixedSample += (static_cast<float>(*(m_playingSample.currentSample++)) / 16384.0f) * 0.5f;

                    if (m_playingSample.currentSample == m_playingSample.sample->sampleEnd) {
                        m_playingSample.currentSample = nullptr;
                        m_playingSample.sample = nullptr;
                    }
                }
            }
        }

        *(outputSamples++) = static_cast<audiosample_t>(mixedSample * 16384.0f);
    } while(--totalSamples > 0);
}

auto Nedrysoft::Audio::transferSamples(SDL_AudioSpec *audioSpec, audiosample_t *outputSamples, void *inputSamples, int totalSamples) -> bool {
    if (audioSpec->format == AUDIO_S32LSB) {
        auto sourceSamples = reinterpret_cast<int32_t *>(inputSamples);

        do {
            *(outputSamples++) = static_cast<audiosample_t >((*(sourceSamples++)) >> 16);
        } while(--totalSamples > 0);
    } else if (audioSpec->format == AUDIO_S16LSB) {
        memcpy(outputSamples, inputSamples, totalSamples * sizeof(audiosample_t));
    } else {
       assert(false);
    }

    return true;
}

auto Nedrysoft::Audio::playSequence(Nedrysoft::SoundId introId, Nedrysoft::SoundId loopId, Nedrysoft::SoundId outtroId) -> void {
    m_playingSequence.state = Nedrysoft::Idle;

    m_playingSequence.sequence.introStart = m_samples[static_cast<int>(introId)].sampleStart;
    m_playingSequence.sequence.introEnd = m_samples[static_cast<int>(introId)].sampleEnd;
    m_playingSequence.sequence.loopStart = m_samples[static_cast<int>(loopId)].sampleStart;
    m_playingSequence.sequence.loopEnd = m_samples[static_cast<int>(loopId)].sampleEnd;
    m_playingSequence.sequence.outtroStart = m_samples[static_cast<int>(outtroId)].sampleStart;
    m_playingSequence.sequence.outtroEnd = m_samples[static_cast<int>(outtroId)].sampleEnd;

    m_playingSequence.sequence.start = m_playingSequence.sequence.introStart;
    m_playingSequence.sequence.end = m_playingSequence.sequence.outtroEnd;

    m_playingSequence.currentSample = m_playingSequence.sequence.start;

    m_playingSequence.state = Nedrysoft::PlayingIntro;
}

auto Nedrysoft::Audio::endSequence() -> void {
    if ((m_playingSequence.state == Nedrysoft::Idle) || (m_playingSequence.state >= Nedrysoft::Ending)) {
        return;
    }

    m_playingSequence.state = Nedrysoft::Ending;
}

auto Nedrysoft::Audio::disableAudio(bool disabled ) -> void {
    m_audioDisabled = disabled;
}