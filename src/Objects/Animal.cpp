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

#include "Animal.h"

#include "Animation.h"
#include "AnimationPlayer.h"
#include "Animations.h"
#include "Audio.h"
#include "Camera.h"
#include "FS.h"
#include "GameRenderer.h"
#include "Hud.h"
#include "Object.h"
#include "RingsFactory.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>

auto constexpr Gravity = 0.21875f;

Nedrysoft::Animations *Nedrysoft::Animal::m_animations = nullptr;

std::random_device Nedrysoft::Animal::m_randomDevice;
std::mt19937 Nedrysoft::Animal::m_randomGenerator(Nedrysoft::Animal::m_randomDevice());
std::uniform_int_distribution<int> Nedrysoft::Animal::m_dist(0, 1);

std::map<std::string, std::vector<Nedrysoft::Animal::AnimalData>> Nedrysoft::Animal::m_data;

auto Nedrysoft::Animal::update() -> void {
}

auto Nedrysoft::Animal::initialise() -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_animations = Animations::load(gameRenderer->renderer(), "./data/art/objects/animals/animals.json");

    auto configStream = Nedrysoft::FS::IOS("./data/art/objects/animals/config.json");

    nlohmann::json jsonObject = nlohmann::json::parse(configStream, nullptr, false);

    if (jsonObject.is_discarded()) {
        return;
    }

    auto typesArray = jsonObject["types"];

    std::map<std::string, AnimalData> typeMap;

    for (auto type: typesArray) {
        AnimalData data = {};

        auto id = type["id"].get<std::string>();

        data.xSpeed = type["xSpeed"].get<float>();
        data.ySpeed = type["ySpeed"].get<float>();
        data.name = type["id"].get<std::string>();
        data.type = toType(data.name);

        typeMap[id] = data;
    }

    auto mapArray = jsonObject["map"];

    for (auto map: mapArray) {
        auto level = map["level"].get<std::string>();
        auto types = map["types"];

        std::vector<AnimalData> animalData;

        for (auto &type : types) {
            auto id = type.get<std::string>();

            if (typeMap.count(type)) {
                animalData.push_back(typeMap[id]);
            }
        }

        m_data[level] = animalData;
    }
}

Nedrysoft::Animal::Animal(float x, float y) :
        Object("animal", 0, x, y, false, false, false),
        m_animal() {

    m_state = AnimalState::Falling;

    auto randomAnimalIndex = m_dist(m_randomGenerator);

    m_animal = m_data["ghz"][randomAnimalIndex];

    m_animations->start(m_animal.name + "-drop", &m_animation);

    m_xSpeed = 0.0f;
    m_ySpeed = m_animal.ySpeed;
}

auto Nedrysoft::Animal::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    SDL_Rect objectRect;
    SDL_Rect screenRect;
    SDL_Rect resultRect;

    m_animation.next();

    m_ySpeed += Gravity;

    m_x += m_xSpeed;
    m_y += m_ySpeed;

    screenRect.x = static_cast<int>(camera->position().x());
    screenRect.y = static_cast<int>(camera->position().y());
    screenRect.w = Nedrysoft::GameRenderer::getInstance()->viewportWidth();
    screenRect.h = Nedrysoft::GameRenderer::getInstance()->viewportHeight();

    objectRect.x = static_cast<int>(m_x);
    objectRect.y = static_cast<int>(m_y);
    objectRect.w = static_cast<int>(m_animation.width());
    objectRect.h = static_cast<int>(m_animation.height());

    if (!SDL_IntersectRect(&objectRect, &screenRect, &resultRect)) {
        return true;
    }

    float distance;

    switch(m_state) {
        case AnimalState::Falling: {
            if (m_ySpeed > 0) {
                if (tileMap->findDown(Vector(m_x, m_y + (m_animation.height() / 2)), &distance)) {
                    if (distance <= 0) {
                        m_ySpeed = m_animal.ySpeed;
                        m_xSpeed = m_animal.xSpeed;

                        m_y += distance;

                        m_state = AnimalState::Bouncing;

                        m_animations->start(m_animal.name + "-flap", &m_animation);
                    }
                }
            }

            break;
        }

        case AnimalState::Bouncing: {
            if (m_ySpeed > 0) {
                if (tileMap->findDown(Vector(m_x, m_y + (m_animation.height() / 2)), &distance)) {
                    if (distance <= 0) {
                        m_ySpeed = m_animal.ySpeed;

                        m_y += distance;
                    }
                }
            }

            break;
        }
    }

    return false;
}

auto Nedrysoft::Animal::render(Camera *camera, ObjectRenderPriority priority) -> void {
    SDL_Rect dest;

    if (priority != ObjectRenderPriority::PriorityOnly) {
        return;
    }

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Texture *texture;
    Vector origin;

    texture = m_animation.texture(&dest.w, &dest.h, &origin);

    assert(texture != nullptr);

    dest.x = static_cast<int>(m_x - origin.x() - camera->position().x());
    dest.y = static_cast<int>(m_y - origin.y() - camera->position().y());

    SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, SDL_FLIP_HORIZONTAL);
}

auto Nedrysoft::Animal::spawn(Camera *camera) -> bool {
    return true;
}

auto Nedrysoft::Animal::toTypeString(AnimalType type) -> std::string {
    switch(type) {
        case Blackbird: {
            return "blackbird";
        }

        case Chicken: {
            return "chicken";
        }

        case Flicky: {
            return "flicky";
        }

        case Pig: {
            return "pig";
        }

        case Rabbit: {
            return "rabbit";
        }

        case Seal: {
            return "seal";
        }

        case Squirrel: {
            return "squirrel";
        }

        default: {
            break;
        }
    }

    assert(false);

    return "unknown";
}

auto Nedrysoft::Animal::toType(const std::string &name) -> AnimalType {
    if (name == "blackbird") {
        return AnimalType::Blackbird;
    } else if (name == "chicken") {
        return AnimalType::Chicken;
    } else if (name == "flicky") {
        return AnimalType::Flicky;
    } else if (name == "pig") {
        return AnimalType::Pig;
    } else if (name == "rabbit") {
        return AnimalType::Rabbit;
    } else if (name == "seal") {
        return AnimalType::Seal;
    } else if (name == "squirrel") {
        return AnimalType::Squirrel;
    }

    assert(false);

    return AnimalType::Unknown;
}