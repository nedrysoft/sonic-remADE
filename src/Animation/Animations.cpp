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

#include "Animations.h"

#include "Animation.h"
#include "AnimationPlayer.h"
#include "AnimationSprite.h"
#include "AnimationStep.h"
#include "FS.h"
#include "Structs.h"
#include "Utils.h"

#include <fstream>
#include <iostream>

constexpr auto DefaultAnimationFrameDuration = 24;

auto Nedrysoft::Animations::load(SDL_Renderer *renderer, const std::string &filename) -> Nedrysoft::Animations * {
    auto animations = new Nedrysoft::Animations;

    std::ifstream tilemapStream(filename);

    nlohmann::json jsonObject = nlohmann::json::parse(tilemapStream, nullptr, false);

    if (jsonObject.is_discarded()) {
        std::cout << color::rize("[unable to load animation]", "Red", "Default", "Bold") +
                                 debugValue("filename", filename, "Cyan", "Green", "Bold") <<
                                 std::endl;

        return nullptr;
    }

    auto spritesArray = jsonObject["sprites"];

    for (auto &sprite : spritesArray) {
        auto id = sprite["id"].get<std::string>();
        auto imageFilename = std::filesystem::path(filename).parent_path().append(sprite["image"].get<std::string>()).string();
        auto originX = static_cast<float>(sprite["origin"]["x"].get<int>());
        auto originY = static_cast<float>(sprite["origin"]["y"].get<int>());

        if (!std::filesystem::exists(imageFilename)) {
            std::cout << color::rize("[unable to load image]", "Red", "Default", "Bold") +
                                     debugValue("filename", imageFilename, "Cyan", "Green", "Bold") <<
                                     std::endl;

            continue;
        }

        Magick::Image image;

        image.magick("PNG");

        image.read(FS::BLOB(imageFilename));

        Magick::Pixels pixelData(image);

        auto sourcePixels = reinterpret_cast<uint8_t *>(pixelData.get(0, 0,  image.size().width(),  image.size().height()));

        auto texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            static_cast<int>(image.size().width()),
            static_cast<int>(image.size().height()));

        uint8_t *destinationPixels;
        int destinationStride;

        SDL_LockTexture(texture, nullptr, reinterpret_cast<void **>(&destinationPixels), &destinationStride);

        assert(destinationStride == image.size().width() * sizeof(uint32_t));

        memcpy(destinationPixels, sourcePixels, image.size().width() * image.size().height() * sizeof(uint32_t));

        SDL_UnlockTexture(texture);

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        auto newSprite = new AnimationSprite(
            id,
            Nedrysoft::Vector(originX, originY),
            static_cast<int>(image.size().width()),
            static_cast<int>(image.size().height()),
            texture
        );

        animations->m_sprites.push_back(newSprite);

        animations->m_spriteMap[id] = newSprite;
    }

    auto animationsArray = jsonObject["animations"];

    for(auto animation : animationsArray) {
        auto newAnimation = new Animation;

        if (animation.contains("speed")) {
            newAnimation->setSpeed(animation["speed"]);
        } else {
            newAnimation->setSpeed(DefaultAnimationFrameDuration);
        }

        std::vector<AnimationStep *> script;

        for (auto item : animation["script"]) {
            AnimationStep *step;

            if (item.contains("show")) {
                step = new AnimationShowStep(item["show"].get<std::string>(), newAnimation);
            } else if (item.contains("next")) {
                step = new AnimationNextStep(item["next"].get<int>(), newAnimation);
            } else if (item.contains("change")) {
                step = new AnimationChangeStep(item["change"].get<std::string>(), newAnimation);
            } else {
                continue;
            }

            script.push_back(step);
        }

        newAnimation->setScript(script);

        newAnimation->setName(animation["id"].get<std::string>());

        animations->m_animations.push_back(newAnimation);
    }

    /**
     * we now go back ground the animation script, we do it in two parts because the change type field can only
     * be fully set-up after the entire set of animations has been created as it may refer to an animation that
     * appears later in the load order.
     */

    for (auto animation : animations->m_animations) {
        for (auto item : animation->script()) {
            item->update(animations);
        }
    }

    return animations;
}

auto Nedrysoft::Animations::sprite(const std::string &id) -> AnimationSprite * {
    if (m_spriteMap.count(id)) {
        return m_spriteMap[id];
    }

    return nullptr;
}

auto Nedrysoft::Animations::sprite(int index) -> AnimationSprite * {
    if (index < m_spriteMap.size()) {
        return m_sprites[index];
    }

    return nullptr;
}

auto Nedrysoft::Animations::find(const std::string &name) -> Animation * {
    for (auto animation : m_animations) {
        for (auto &item : animation->script()) {
            if (animation->name() == name) {
                return animation;
            }
        }
    }

    return nullptr;
}

auto Nedrysoft::Animations::start(
        const std::string &name,
        AnimationPlayer *runningAnimation,
        const FrameDurationFunction &frameDuration) -> bool {

    auto animation = find(name);

    if (animation) {
        runningAnimation->setAnimation(animation);
        runningAnimation->setDuration(frameDuration);

        return true;
    }

    return false;
}

auto Nedrysoft::Animations::start(
        const std::string &name,
        AnimationPlayer *runningAnimation,
        int frameDuration) -> bool {

    auto animation = find(name);

    if (animation) {
        runningAnimation->setAnimation(animation);
        runningAnimation->setDuration(frameDuration);

        return true;
    }

    return false;
}

auto Nedrysoft::Animations::index(const std::string &id) -> int {
    int index = 0;

    for(auto animation : m_animations) {
        if (animation->name() == id) {
            return index;
        }

        index++;
    }

    return -1;
}

auto Nedrysoft::Animations::start(const std::list<std::string> &group, AnimationPlayer *runningAnimation, const FrameDurationFunction &durationFunction) -> bool {
    std::list<Animation *> animations;

    for (auto name : group) {
        auto animation = find(name);

        if (!animation) {
            return false;
        }

        animations.push_back(animation);
    }

    runningAnimation->setAnimations(animations);
    runningAnimation->setDuration(durationFunction);

    return true;
}
