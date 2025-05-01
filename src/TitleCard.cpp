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

#include "TitleCard.h"

#include "GameRenderer.h"
#include "Image.h"
#include <cassert>
#include <iostream>

auto constexpr CardSpeed = 16;

int Nedrysoft::TitleCard::m_imageY[4] = {
    -48,
    -28,
    -22,
    -32
};

Nedrysoft::TitleCard::TitleCard() :
        m_positions{0},
        m_state(Finished) {

}
/*
Card_PosData:	; y pos, x pos
		dc.w 0,	$120					; GREEN HILL
		dc.w -$104, $13C				; ZONE
		dc.w $414, $154					; ACT x
		dc.w $214, $154					; oval
		dc.w 0,	$120					; LABYRINTH
		dc.w -$10C, $134
		dc.w $40C, $14C
		dc.w $20C, $14C
		dc.w 0,	$120					; MARBLE
		dc.w -$120, $120
		dc.w $3F8, $138
		dc.w $1F8, $138
		dc.w 0,	$120					; STAR LIGHT
		dc.w -$104, $13C
		dc.w $414, $154
		dc.w $214, $154
		dc.w 0,	$120					; SPRING YARD
		dc.w -$FC, $144
		dc.w $41C, $15C
		dc.w $21C, $15C
		dc.w 0,	$120					; SCRAP BRAIN
		dc.w -$FC, $144
		dc.w $41C, $15C
		dc.w $21C, $15C
		dc.w 0,	$120					; FINAL
		dc.w -$11C, $124
		dc.w $3EC, $3EC
		dc.w $1EC, $12C
		endm
*/
auto Nedrysoft::TitleCard::start(TitleCardZone zone, TitleCardAct act) -> void {
    if (m_state != Finished) {
        return;
    }

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_levelImages[TitleCardZone::GreenHill] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-title-card-ghz.png", "PNG");
    m_levelImages[TitleCardZone::Labyrinth] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-title-card-lz.png", "PNG");
    m_levelImages[TitleCardZone::Marble] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-title-card-mz.png", "PNG");
    m_levelImages[TitleCardZone::ScrapBrain] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-title-card-sbz.png", "PNG");
    m_levelImages[TitleCardZone::StarLight] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-title-card-slz.png", "PNG");
    m_levelImages[TitleCardZone::SpringYard] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-title-card-syz.png", "PNG");

    m_actImages[TitleCardAct::One] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-act1.png", "PNG");
    m_actImages[TitleCardAct::Two] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-act2.png", "PNG");
    m_actImages[TitleCardAct::Three] = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-act3.png", "PNG");

    m_zoneImage = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-title-card-zone.png", "PNG");
    m_ovalImage = new Nedrysoft::Image(gameRenderer->renderer(), "./data/art/objects/title-cards/title-cards-title-card-oval.png", "PNG");

	m_cardData[0] = {0, 288, 1};
	m_cardData[1] = {-260, 316, 1};
    m_cardData[2] = {1044, 340, -1};
    m_cardData[3] = {532, 340, -1};

    m_zoneId = zone;
    m_actId = act;

    m_state = Nedrysoft::In;
    m_alpha = 0xFF;

    for (auto &position : m_positions) {
        position = 0;
    }
}

auto Nedrysoft::TitleCard::update() -> void {
    switch(m_state) {
        case In: {
            bool finishedMoving = true;

            for (int i = 0; i < 4; i++) {
                if (m_positions[i] < abs(m_cardData[i].end - m_cardData[i].start)) {
                    m_positions[i] += 16;

                    finishedMoving = false;
                } else {
                    m_positions[i] = abs(m_cardData[i].end - m_cardData[i].start);
                }
            }

            if (finishedMoving) {
                m_timer = 15;
                m_state = PreFade;
            }

            break;
        }

        case PreFade: {
            m_timer--;

            if (m_timer == 0) {
                m_state = Fade;
            }
            break;
        }

        case Fade: {
            m_alpha -= 8;

            if (m_alpha < 0) {
                m_state = PostFade;
                m_timer = 50;
                m_alpha = 0;
            }

            break;
        }

        case PostFade: {
            m_timer--;

            if (m_timer == 0) {
                m_state = Out;
            }

            break;

        }

        case Out: {
            bool finishedMoving = true;

            for (auto &position : m_positions) {
                if (position > 0) {
                    position -= 32;

                    finishedMoving = false;
                }
            }

            if (finishedMoving) {
                m_state = Finished;
            }

            break;
        }

        case Finished: {
            break;
        }
    }
}

auto Nedrysoft::TitleCard::render() -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect dest;

    if (m_state == Finished) {
        return;
    }

    dest.x = 0;
    dest.y = 0;
    dest.w = gameRenderer->viewportWidth();
    dest.h = gameRenderer->viewportHeight();

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0x00, 0x00, 0x00, m_alpha);

    SDL_RenderFillRect(gameRenderer->renderer(), &dest);

    renderImage(TitleCardImage::Oval);
    renderImage(TitleCardImage::Level);
    renderImage(TitleCardImage::Zone);
    renderImage(TitleCardImage::Act);

    update();
}

auto Nedrysoft::TitleCard::renderImage(int id) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect dest;

    SDL_Texture *texture;
    Image *image;

    switch(id) {
        case Oval: {
            image = m_ovalImage;

            break;
        }

        case Zone: {
            image = m_zoneImage;

            break;
        }

        case Level: {
            image = m_levelImages[m_zoneId];

            break;
        }

        case Act: {
            image = m_actImages[m_actId];

            break;
        }

        default: {
            assert(false);
        }
    }

    texture = image->texture();

    int width = image->width();
    int height = image->height();

    int x = m_cardData[id].start + (m_cardData[id].sign * m_positions[id]);

    dest.x = x - (width / 2) - (gameRenderer->viewportWidth() / 2) + CardSpeed;
    dest.y = m_imageY[id] - (height / 2) + (gameRenderer->viewportHeight() / 2) + CardSpeed;
    dest.w = width;
    dest.h = height;

    SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, 0, nullptr, SDL_FLIP_NONE);
}

auto Nedrysoft::TitleCard::finished() -> bool {
        return m_state == Finished;
}
