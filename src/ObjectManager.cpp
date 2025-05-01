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

#include "ObjectsManager.h"

#include "Animal.h"
#include "BuzzBomber.h"
#include "BuzzBomberFactory.h"
#include "Bridge.h"
#include "BridgeFactory.h"
#include "Camera.h"
#include "Chopper.h"
#include "ChopperFactory.h"
#include "CollapsingLedge.h"
#include "CollapsingLedgeFactory.h"
#include "CrabMeat.h"
#include "CrabMeatFactory.h"
#include "EdgeWall.h"
#include "EdgeWallFactory.h"
#include "GameRenderer.h"
#include "GiantRing.h"
#include "GiantRingFactory.h"
#include "HiddenBonus.h"
#include "HiddenBonusFactory.h"
#include "Lamppost.h"
#include "LamppostFactory.h"
#include "Monitor.h"
#include "MonitorFactory.h"
#include "MotoBug.h"
#include "MotoBugFactory.h"
#include "Newtron.h"
#include "NewtronFactory.h"
#include "Platform.h"
#include "PlatformFactory.h"
#include "Points.h"
#include "PurpleRock.h"
#include "PurpleRockFactory.h"
#include "Rings.h"
#include "RingsFactory.h"
#include "Scenery.h"
#include "SceneryFactory.h"
#include "Signpost.h"
#include "SignpostFactory.h"
#include "Spikes.h"
#include "SpikesFactory.h"
#include "Spring.h"
#include "SpringFactory.h"
#include "WaterfallSound.h"
#include "WaterfallSoundFactory.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <nlohmann/json.hpp>
#include <set>

constexpr auto spawnWindowWidth = 256;
constexpr auto spawnWindowHeight = 256;

auto Nedrysoft::ObjectsManager::getInstance() -> ObjectsManager * {
    static ObjectsManager instance;

    return &instance;
}

auto Nedrysoft::ObjectsManager::load(const std::string &filename) -> bool {
    std::set<std::string> objectsSet;

    m_factories["rings"] = Nedrysoft::RingsFactory::initialise();
    m_factories["bridge"] = Nedrysoft::BridgeFactory::initialise();
    m_factories["scenery"] = Nedrysoft::SceneryFactory::initialise();
    m_factories["purple-rock"] = Nedrysoft::PurpleRockFactory::initialise();
    m_factories["monitor"] = Nedrysoft::MonitorFactory::initialise();
    m_factories["collapsing-ledge"] = Nedrysoft::CollapsingLedgeFactory::initialise();
    m_factories["edge-wall"] = Nedrysoft::EdgeWallFactory::initialise();
    m_factories["spikes"] = Nedrysoft::SpikesFactory::initialise();
    m_factories["spring"] = Nedrysoft::SpringFactory::initialise();
    m_factories["platform"] = Nedrysoft::PlatformFactory::initialise();
    m_factories["enemy-motobug"] = Nedrysoft::MotoBugFactory::initialise();
    m_factories["enemy-chopper"] = Nedrysoft::ChopperFactory::initialise();
    m_factories["enemy-buzzbomber"] = Nedrysoft::BuzzBomberFactory::initialise();
    m_factories["enemy-crabmeat"] = Nedrysoft::CrabMeatFactory::initialise();
    m_factories["enemy-newtron"] = Nedrysoft::NewtronFactory::initialise();
    m_factories["lamppost"] = Nedrysoft::LamppostFactory::initialise();
    m_factories["waterfall-sound"] = Nedrysoft::WaterfallSoundFactory::initialise();
    m_factories["signpost"] = Nedrysoft::SignpostFactory::initialise();
    m_factories["giant-ring"] = Nedrysoft::GiantRingFactory::initialise();
    m_factories["hidden-bonus"] = Nedrysoft::HiddenBonusFactory::initialise();

    /**
     * some dynamic objects are spawned directly via another object and therefore there is no need for a factory
     * class to create them, so we need to manually run the initialise method on the class to ensure that the
     * object instances have access to any required data.
     */

    Nedrysoft::Animal::initialise();
    Nedrysoft::Points::initialise();

    std::ifstream tilemapStream(filename);

    nlohmann::json objectArray = nlohmann::json::parse(tilemapStream, nullptr, false);

    if (objectArray.is_discarded()) {
        std::cout << "error loading json." << std::endl;

        return false;
    }

    try {
        for (auto object : objectArray) {
            auto subType = object["subType"].get<int>();
            auto flipped = object["flipped"].get<bool>();
            auto mirrored = object["mirrored"].get<bool>();
            auto type = object["type"].get<std::string>();
            auto rememberState = object["rememberState"].get<bool>();

            auto x = static_cast<float>(object["x"].get<int>());
            auto y = static_cast<float>(object["y"].get<int>());

            if (m_factories.count(type)) {
                auto newObject = m_factories[type]->create(
                    subType,
                    x,
                    y,
                    rememberState,
                    mirrored,
                    flipped
                );

                m_objects.push_back(newObject);
            } else {
                objectsSet.insert(type);
            }
        }
    } catch (nlohmann::json::exception &e) {
        return false;
    }

    std::set<std::string> objectTypes;

    for (auto object : m_objects) {
        objectTypes.insert(object->type());
    }

    objectsSet.erase("sonic");

    //std::cout << "loaded " << m_objects.size() << " objects, there are " << objectTypes.size() << " object types in the level." << std::endl;

    std::cout << color::rize("[loaded objects]", "Green", "Default", "Bold") +
                             debugValue("total objects", std::to_string(m_objects.size()), "Cyan", "Green", "Bold") <<
                             debugValue("types used", std::to_string(objectTypes.size()), "Cyan", "Green", "Bold") <<
                             std::endl;

    if (!objectsSet.empty()) {

        for (const auto &objectType : objectsSet) {
            std::cout << color::rize("[unknown object]", "Red", "Default", "Bold") +
                                     debugValue("type", objectType, "Cyan", "Green", "Bold") <<
                                     std::endl;
        }
    }

    m_firstObject = m_objects.begin();

    return true;
}

auto Nedrysoft::ObjectsManager::update(TileMap *tileMap, Camera *camera, class Sonic *sonic) -> void {
    static int lastVisibleSpawnableCount = 0;

    despawnObjects(camera, sonic);
    spawnObjects(camera, sonic);

    auto totalVisibleSpawnableCount = m_spawnedObjects.size();

    if (totalVisibleSpawnableCount != lastVisibleSpawnableCount) {
        std::cout << color::rize("[spawned objects changed]", "Yellow", "Default", "Bold") +
            debugValue("now", std::to_string(totalVisibleSpawnableCount), "Cyan", "Green", "Bold") +
            debugValue("was", std::to_string(lastVisibleSpawnableCount), "Cyan", "Green", "Bold") <<
            std::endl;

        lastVisibleSpawnableCount = static_cast<int>(totalVisibleSpawnableCount);
    }

    /**
     * some objects have common animations that is synced across all instances of the objects (rings for
     * example), so each factory includes an update() method that can be used to update any information
     * across all objects once per frame
     */

    for (const auto &factory : m_factories) {
        factory.second->update();
    }

    /**
     * once the class update has been done, we can perform any object specific updates that need to be done.
     */

    for (auto objectIterator = m_spawnedObjects.begin(), last = m_spawnedObjects.end(); objectIterator != last; ) {
        auto object = *objectIterator;

        if (object->update(tileMap, camera, sonic)) {
            objectIterator = m_spawnedObjects.erase(objectIterator);
        } else {
            objectIterator++;
        }
    }

    for (auto objectIterator = m_dynamicObjects.begin(), last = m_dynamicObjects.end(); objectIterator != last; ) {
        auto object = *objectIterator;

        if (object->update(tileMap, camera, sonic)) {
            objectIterator = m_dynamicObjects.erase(objectIterator);
        } else {
            objectIterator++;
        }
    }
}

auto Nedrysoft::ObjectsManager::spawnObjects(Camera *camera, class Sonic *sonic) -> void {
    N_UNUSED(sonic)

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto spawnViewportLeft = std::max(0.0f, camera->position().x() - spawnWindowWidth);
    auto spawnViewportRight = camera->position().x() + static_cast<float>(gameRenderer->viewportWidth()) + spawnWindowWidth;
    auto spawnViewportTop = std::max(0.0f, camera->position().y() - spawnWindowHeight);
    auto spawnViewportBottom = camera->position().y() + static_cast<float>(gameRenderer->viewportHeight()) + spawnWindowHeight;

    for (auto object : m_objects) {
        if (m_spawnedObjects.count(object)) {
            continue;
        }

        if ( (object->x() > spawnViewportLeft) &&
             (object->x() < spawnViewportRight) &&
             (object->y() > spawnViewportTop) &&
             (object->y() < spawnViewportBottom) ) {

            if (object->spawn(camera)) {
                std::cout << color::rize("[spawned]", "Yellow", "Default", "Bold") +
                             debugValue("type", object->type(), "Cyan", "Green", "Bold") <<
                             std::endl;

                m_spawnedObjects.insert(object);
            }
        }
    }
}

auto Nedrysoft::ObjectsManager::despawnObjects(Camera *camera, class Sonic *sonic) -> void {
    N_UNUSED(sonic)

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto spawnViewportLeft = std::max(0.0f, camera->position().x() - spawnWindowWidth);
    auto spawnViewportRight = camera->position().x() + static_cast<float>(gameRenderer->viewportWidth()) + spawnWindowWidth;
    auto spawnViewportTop = std::max(0.0f, camera->position().y() - spawnWindowHeight);
    auto spawnViewportBottom = camera->position().y() + static_cast<float>(gameRenderer->viewportHeight()) + spawnWindowHeight;

    for (auto objectIterator = m_spawnedObjects.begin(), last = m_spawnedObjects.end(); objectIterator != last; ) {
        auto object = *objectIterator;

        /**
         * check if the object is inside the current "spawn viewport", if it is, then we ignore it as
         * we still consider it to be "running".
         */

        if ( (object->x() >= spawnViewportLeft) &&
             (object->x() <= spawnViewportRight) &&
             (object->y() >= spawnViewportTop) &&
             (object->y() <= spawnViewportBottom)) {

            objectIterator++;

            continue;
        }

        std::cout << color::rize("[despawned]", "Yellow", "Default", "Bold") +
            debugValue("type", object->type(), "Cyan", "Green", "Bold") <<
            std::endl;

        objectIterator = m_spawnedObjects.erase(objectIterator);
    }
}

auto Nedrysoft::ObjectsManager::updateFirstObject(Camera *camera) -> void {
    /**
     * these first two cases stop us needlessly oscillating around the start and end of the list when the
     * the camera is either to the left of the first object or to the right of the last object.
     */
    if (m_firstObject == m_objects.begin()) {
        if (m_firstObject != m_objects.end()) {
            if ( camera->position().x() < (*(m_firstObject + 1))->x() ) {
                m_firstObject = m_objects.begin();

                return;
            }
        }
    }

    if (m_firstObject == m_objects.end()) {
        if (m_firstObject != m_objects.begin()) {
            if ( camera->position().x() > (*(m_firstObject - 1))->x() ) {
                m_firstObject = m_objects.end();

                return;
            }
        }

        while(--m_firstObject != m_objects.begin()) {
            if ((*m_firstObject)->x() >= camera->position().x()) {
                continue;
            }

            return;
        }

        return;
    }

    /**
     * we are somewhere between the beginning and the end of the objects, so we need to move the iterator so that
     * it is positioned to the object that is directly to the right of the viewports X.
     */

    if ((*m_firstObject)->x() < camera->position().x()) {
        while(m_firstObject != m_objects.end()) {
            if ((*m_firstObject)->x() >= camera->position().x()) {
                return;
            }

            m_firstObject++;
        }
    } else {
        do {
            if ((*m_firstObject)->x() < camera->position().x()) {
                m_firstObject++;

                return;
            }

            if (m_firstObject == m_objects.begin()) {
                return;
            }

            m_firstObject--;
        } while(m_firstObject != m_objects.begin());
    }
}

auto Nedrysoft::ObjectsManager::render(Camera *camera, Nedrysoft::ObjectRenderPriority priority) -> void {
    for (auto object : m_spawnedObjects) {
        object->render(camera, priority);
    }

    for (auto object : m_dynamicObjects) {
        object->render(camera, priority);
    }
}

auto Nedrysoft::ObjectsManager::renderCollisions(Camera *camera) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect dest;

    auto viewportLeft = camera->position().x();
    auto viewportRight = viewportLeft + static_cast<float>(gameRenderer->viewportWidth());
    auto viewportTop = camera->position().y();
    auto viewportBottom = viewportTop + static_cast<float>(gameRenderer->viewportHeight());

    auto iterator = m_firstObject;

    while(iterator != m_objects.end()) {
        auto object = (*iterator++);
        
        if ((object->x() < viewportLeft) || (object->x() > viewportRight)) {
            continue;
        }

        if ((object->y() < viewportTop) || (object->y() > viewportBottom)) {
            continue;
        }

        object->renderCollision(camera);
    }
}

auto Nedrysoft::ObjectsManager::visibleSpawnable(Camera *camera) -> int {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto total = 0;

    auto viewportLeft = camera->position().x();
    auto viewportRight = viewportLeft + static_cast<float>(gameRenderer->viewportWidth());
    auto viewportTop = camera->position().y();
    auto viewportBottom = viewportTop + static_cast<float>(gameRenderer->viewportHeight());

    auto iterator = m_firstObject;

    while(iterator != m_objects.end()) {
        auto object = (*iterator++);

        if (object->x() > viewportRight) {
            break;
        }

        if ((object->y() < viewportTop) || (object->y() > viewportBottom)) {
            continue;
        }

        total++;
    }

    return total;
}

auto Nedrysoft::ObjectsManager::addDynamicObject(Object *object) -> void {
    m_dynamicObjects.push_back(object);
}

auto Nedrysoft::ObjectsManager::reset() -> void {
    for (auto object : m_objects) {
        object->reset();
    }

    //m_dynamicObjects.erase();
}
