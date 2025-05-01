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

#include "CollapsingLedge.h"

#include "Audio.h"
#include "Camera.h"
#include "GameRenderer.h"
#include "Input.h"
#include "Object.h"
#include "Sonic.h"
#include "Utils.h"

#include <SDL2/SDL.h>
#include <fstream>

//#define DEBUG_DRAW_COLLISION_BOX
//#define DEBUG_DISABLE_COLLAPSE

/**
 * constants used by this object
 */

auto constexpr CollapseWaitTime = 7;                    //!< The delay from Sonic standing on the ledge to the start of it collapsing
auto constexpr Gravity = 0.21875f;                      //!< The gravity coefficient

/**
 * static member initialisation
 */

auto Nedrysoft::CollapsingLedge::m_image = reinterpret_cast<Image *>(NULL);

auto Nedrysoft::CollapsingLedge::m_heights = std::vector<int>();
auto Nedrysoft::CollapsingLedge::m_timings = std::vector<int>();

auto Nedrysoft::CollapsingLedge::m_sections = std::vector<Nedrysoft::CollapsingLedge::Section>();

auto Nedrysoft::CollapsingLedge::m_hitboxHeightRadius = 0;
auto Nedrysoft::CollapsingLedge::m_hitboxWidthRadius = 0;
auto Nedrysoft::CollapsingLedge::m_originX = 0;
auto Nedrysoft::CollapsingLedge::m_originY = 0;
auto Nedrysoft::CollapsingLedge::m_blockWidth = 0;
auto Nedrysoft::CollapsingLedge::m_blockHeight = 0;

auto Nedrysoft::CollapsingLedge::update() -> void {

}

auto Nedrysoft::CollapsingLedge::initialise() -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    std::ifstream objectStream("./data/art/objects/ghz-collapsing-ledge/ghz-collapsing-ledge.json");

    nlohmann::json jsonObject = nlohmann::json::parse(objectStream, nullptr, false);

    if (jsonObject.is_discarded()) {
        std::cout << "error loading json." << std::endl;

        return;
    }

    m_image = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/ghz-collapsing-ledge/" + jsonObject["image"].get<std::string>(), "PNG");

    auto heights = jsonObject["heights"];

    for (auto &height : heights) {
        m_heights.push_back(height.get<int>());
    }

    m_hitboxHeightRadius = m_image->height() / 2;
    m_hitboxWidthRadius = m_image->width() / 2;

    m_originX = jsonObject["origin"]["x"].get<int>();
    m_originY = jsonObject["origin"]["y"].get<int>();

    for (auto sectionObject : jsonObject["sections"]) {
        Section section = {};

        section.height = sectionObject["height"].get<int>();
        section.width = sectionObject["width"].get<int>();
        section.left = sectionObject["left"].get<int>();
        section.top = sectionObject["top"].get<int>();
        section.flipped = sectionObject["flipped"].get<bool>();
        section.mirrored = sectionObject["mirrored"].get<bool>();
        section.priority = sectionObject["priority"].get<bool>();
        section.paletteLine = sectionObject["paletteLine"].get<int>();
        section.tileIndex = sectionObject["tileIndex"].get<int>();

        m_sections.push_back(section);
    }

    for (auto &timingsObject : jsonObject["fragmentTimings"]) {
        m_timings.push_back(timingsObject.get<int>());
    }

    m_blockWidth = jsonObject["blockSize"]["width"].get<int>();
    m_blockHeight = jsonObject["blockSize"]["height"].get<int>();
}

Nedrysoft::CollapsingLedge::CollapsingLedge(int subType, float x, float y, bool rememberState, bool mirrored, bool flipped) :
        Object("collapsing-ledge", subType, x, y, rememberState, mirrored, flipped) {

    reset();
}

auto Nedrysoft::CollapsingLedge::update(TileMap *tileMap, Camera *camera, Sonic *sonic) -> bool {
    N_UNUSED(tileMap)

    auto viewportHeight = Nedrysoft::GameRenderer::getInstance()->viewportHeight();

    SDL_Rect sonicRect;
    SDL_Rect resultRect;

    if (m_state == State::Collapsed) {
        return true;
    }

    if (m_state == State::Waiting) {
        m_waitTimer--;

        if (!m_waitTimer) {
            m_state = State::Collapsing;
        } else {
            return false;
        }
    }

    /**
     * first up, check if we're collapsing, if we are then we check each fragment in turn, if it has a non
     * zero ySpeed then we increment ySpeed with gravity and then add that to the fragments y coordinate.
     *
     * we then check if the fragments timing is non zero, if it is then we decrement it, we then check if it's
     * zero, if it is then we set ySpeed to gravity starting the collapse for the current fragment.
     */

    if (m_state == State::Collapsing) {
        auto fullyCollapsed = true;

        for (auto &fragment : m_fragments) {
            if (!fragment.active) {
                continue;
            }

            if (fragment.ySpeed != 0.0f) {
                fragment.ySpeed += Gravity;

                fragment.y += fragment.ySpeed;

                if (fragment.surface) {
                    m_surfaceOffsets[fragment.sourceX / 16] += fragment.ySpeed;
                }
            }

            if (fragment.timing) {
                fragment.timing--;

                if (!fragment.timing) {
                    fragment.ySpeed = Gravity;
                }
            }

            if ((static_cast<int>(fragment.y) - (fragment.height / 2)) > (static_cast<int>(camera->position().y()) + viewportHeight)) {
                fragment.active = false;
            } else {
                fullyCollapsed = false;
            }
        }

        if (fullyCollapsed) {
            m_state = State::Collapsed;

            return true;
        }
    }

    /**
     * if Sonic is jumping upwards then we know he can't collide, he can only collide with the platform when he
     * is travelling downwards (either jumping down or falling down).
     */

    if (sonic->isJumpingUp()) {
        return false;
    }

    /**
     * if sonic is dying then he can't collide with anything, so we exit early.
     */

    if (sonic->isDying()) {
        return false;
    }

    /**
     * if sonic is not moving downwards then we can't collide with the ledge.
     */

    if (sonic->verticalSpeed() < 0) {
        return false;
    }

    /**
     * now we check to see if sonic is colliding with the ledge.
     */

    sonicRect = sonic->hitBox();

    if (!SDL_IntersectRect(&m_collisionRect, &sonicRect, &resultRect)) {
        if (sonic->standingOnObject() == this) {
            sonic->setStandingOnObject(nullptr);
        }

        m_collisionState = FirstCollision;

        return false;
    }

    /**
     * if sonic is to the left or right of the ledge we can't make contact, so we exit
     */

    int sonicX = static_cast<int>(sonic->position().x() + (sonic->rect().width() / 2));

    if ((sonicX < m_collisionRect.x) || (sonicX > (m_collisionRect.x + m_collisionRect.w))) {
        return false;
    }

    /**
     * we know that sonics hit box and the ledges hit boxes have intersected, we need to do a few more checks,
     * but we use the height information in the ledge to figure out to figure out where ground level is relative
     * to sonic, as we're an object we need to move Sonic since he only tests against the tilemap.
     */

    int offset = static_cast<int>(sonicX - (static_cast<int>(m_x) - m_hitboxWidthRadius));

    if (offset < 0) {
        offset = 0;
    } if (offset > (m_heights.size() * 2) - 1) {
        offset = (static_cast<int>(m_heights.size()) * 2) - 1;
    }

    if (m_mirrored) {
        offset = (static_cast<int>(m_heights.size()) * 2) - 1 - offset;
    }

    int collapsingOffsets = m_surfaceOffsets[offset / 16];

    int yDistance = collapsingOffsets + static_cast<int>(m_y) - m_heights[offset / 2] - static_cast<int>(sonic->rect().bottom());

    /**
     * we check the initial collision to see if the yDistance is below 16, this prevents us from making contact
     * below the ledge and snapping up to it, we disable the collision tests until we have left the bounding box of the ledge.
     */

    if (m_collisionState & CollisionStates::FirstCollision) {
        if (yDistance <= -16) {
            m_collisionState |= CollisionStates::Disabled;

            return false;
        }

        m_collisionState &= ~CollisionStates::FirstCollision;
    }

    if (!sonic->standingOnObject()) {
        if (yDistance < 0) {
            setStandingOnObject(yDistance + 1,  tileMap, camera, sonic);
        }
    } else {
        if (yDistance < 0) {
            setStandingOnObject(yDistance + 1,  tileMap, camera, sonic);
        } else {
            sonic->setStandingOnObject(nullptr);
        }
    }

    return false;
}

auto Nedrysoft::CollapsingLedge::setStandingOnObject(int distance, TileMap *tileMap, Camera *camera, Sonic *sonic) -> void {
    N_UNUSED(tileMap)
    N_UNUSED(camera)

    sonic->move(0.0f, static_cast<float>(distance + 1));

    sonic->setStandingOnObject(this);

    if (m_state == State::Normal) {
        Nedrysoft::Audio::getInstance()->playSample(2, Nedrysoft::SoundId::Collapse);

#if !defined(DEBUG_DISABLE_COLLAPSE)
        m_state = State::Waiting;
#endif
        m_waitTimer = CollapseWaitTime;
    }
}

auto Nedrysoft::CollapsingLedge::render(Camera *camera, ObjectRenderPriority priority) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    if (priority != ObjectRenderPriority::NonPriorityOnly) {
         return;
    }

    if (m_image == nullptr) {
        return;
    }

    if (m_state == State::Collapsed) {
        return;
    }

    SDL_Rect dest;
    SDL_Rect source;

    /**
     * Our sprite is the full size of the "final" sprite, in the megadrive the sprite is made up of smaller sprites
     * because of the limitations of the hardware, and they make use of this to easily create the collapsing ledge.
     *
     * When we loaded the JSON we read in the sections and generated the necessary data that allows us to very
     * quickly and simply render the sprite as multiple tiles, restoring behaviour similar to the megadrive.  We
     * loop through each of the fragments and simply render sections of the sprite to the target.
     */

    for (auto fragment : m_fragments) {
        if (!fragment.active) {
            continue;
        }

        dest.x = static_cast<int>(round(fragment.x) - camera->position().x() );
        dest.y = static_cast<int>(round(fragment.y) - camera->position().y());
        dest.w = fragment.width;
        dest.h = fragment.height;

        source.x = fragment.sourceX;
        source.y = fragment.sourceY;
        source.w = dest.w;
        source.h = dest.h;

        int flipFlags = SDL_FLIP_NONE;

        flipFlags |= (m_mirrored ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        flipFlags |= (m_flipped ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE);

        SDL_RenderCopyEx(gameRenderer->renderer(), m_image->texture(), &source, &dest, 0, nullptr, static_cast<SDL_RendererFlip>(flipFlags));
    }

#if defined(DEBUG_DRAW_COLLISION_BOX)
    auto collisionBox = m_collisionRect;

    collisionBox.x -= static_cast<int>(camera->position().x());
    collisionBox.y -= static_cast<int>(camera->position().y());

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0x00, 0x00, 0x80);

    SDL_RenderDrawRect(gameRenderer->renderer(), &collisionBox);
#endif
}

auto Nedrysoft::CollapsingLedge::spawn(Camera *camera) -> bool {
    m_collisionState = CollisionStates::FirstCollision;

    return m_state != State::Collapsed;
}

auto Nedrysoft::CollapsingLedge::reset() -> void {
    Object::reset();

    m_collisionRect = {
        .x = static_cast<int>(m_x) - m_hitboxWidthRadius,
        .y = static_cast<int>(m_y) - m_originY,
        .w = m_hitboxWidthRadius * 2,
        .h = m_hitboxHeightRadius * 2
    },
       
     m_state = State::Normal;

    if (m_image == nullptr) {
        return;
    }

    m_fragments.clear();

    /**
     * the collapsing ledge is fragmented, the sections of the image also define how the ledge breaks apart after
     * sonic lands on it, so we use the sections to create the fragments when we create the ledge, we set up
     * the source X and Y along with the X & Y of the tiles which makes life easier when it comes to
     * collapsing the ledge, and also rendering it.
     */

    for (auto section : m_sections) {
        Fragment fragment = {};

        fragment.sourceX = m_originX + section.left;
        fragment.sourceY = m_originY + section.top;
        fragment.width = section.width * m_blockWidth;
        fragment.height = section.height * m_blockHeight;
        fragment.timing = m_timings[m_sections.size() - 1 - m_fragments.size()];
        fragment.active = true;
        fragment.surface = false;

        if (!m_mirrored) {
            fragment.x = static_cast<int>(m_x) + fragment.sourceX - m_originX;
        } else {
            fragment.x = static_cast<int>(m_x) + (m_image->width() - fragment.width - fragment.sourceX) - m_originX;
        }

        fragment.y = m_y + static_cast<float>(fragment.sourceY - m_originY);
        fragment.ySpeed = 0;

        m_fragments.push_back(fragment);
    }

    std::map<int, Fragment *> surfaceFragments;

    for (auto &fragment : m_fragments) {
        int fragmentIndex = fragment.sourceX / 16;

        if (!surfaceFragments.count(fragmentIndex)) {
            surfaceFragments[fragmentIndex] = &fragment;
        } else {
            auto existingFragment = surfaceFragments[fragmentIndex];

            if (existingFragment->y > fragment.y) {
                surfaceFragments[fragmentIndex] = &fragment;
            }
        }

        m_surfaceOffsets[fragmentIndex] = 0;
    }

    for (auto fragment : surfaceFragments) {
        fragment.second->surface = true;
    }
} 
