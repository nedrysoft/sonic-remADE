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

#ifndef NEDRYSOFT_PARALLAXTILEMAP_H
#define NEDRYSOFT_PARALLAXTILEMAP_H

#include "PaletteCycle.h"
#include "Structs.h"
#include "Tile.h"
#include "Tileset.h"

#include <Magick++.h>
#include <SDL2/SDL_render.h>
#include <list>
#include <string>

namespace Nedrysoft {
    enum class ParallaxItemType {
        Undefined,
        Block,
        Lines
    };

    struct ParallaxPalette {
        uint32_t m_data[16 * 4];
    };

    struct ParallaxColour {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;
    };

    class ParallaxItem {
        public:
            ParallaxItem();

            auto name() -> std::string;

            auto adjustOffsets(float x, float y) -> void;
            [[nodiscard]] auto xOffset() const -> float;
            [[nodiscard]] auto yOffset() const -> float;
            [[nodiscard]] auto frameTicks() const -> int;
            [[nodiscard]] auto startY() const-> int;
            [[nodiscard]] auto endY() const -> int;
            [[nodiscard]] auto autoScrollSpeed() const -> float;
            [[nodiscard]] auto textureWidth() const -> int;
            auto textures() -> std::vector<Texture *> &;
            auto type() -> ParallaxItemType;

            auto setName(const std::string &name) -> void;

            auto setXOffset(int offset) -> void;
            auto setYOffset(int offset) -> void;
            auto setFrameTicks(int frameTicks) -> void;
            auto setStartY(int y) -> void;
            auto setEndY(int y) -> void;
            auto setAutoScrollSpeed(float speed) -> void;
            auto setTextureWidth(int width) -> void;
            auto setType(ParallaxItemType type) -> void;

        private:
            std::string m_name;

            ParallaxItemType m_type;

            float m_xOffset;
            float m_yOffset;
            int m_frameTicks;

            int m_startY;
            int m_endY;

            float m_autoScrollSpeed;

            int m_textureWidth;

            std::vector<Texture *> m_textures;
    };

    class ParallaxTileMap {
        public:
            static auto load(const std::string &filename) -> ParallaxTileMap *;

            auto render(const Vector &position) -> void;

            [[nodiscard]] auto widthInPixels() const -> int;
            [[nodiscard]] auto heightInPixels() const -> int;

            [[nodiscard]] auto backgroundRed() const -> uint8_t;
            [[nodiscard]] auto backgroundGreen() const -> uint8_t;
            [[nodiscard]] auto backgroundBlue() const -> uint8_t;
            [[nodiscard]] auto backgroundAlpha() const -> uint8_t;

            auto update() -> void;

            auto setColours(const std::string &name) -> void;

            auto setPalette(const std::string &name, bool updateTextures = false) -> void;
            auto setCycle(const std::string &name, bool updateTextures = false) -> void;
            auto setBackground(const std::string &name, bool updateTextures = false) -> void;

        private:
            ParallaxTileMap();
            ~ParallaxTileMap() = default;

        private:
            Tileset m_tileSet;

            std::vector<std::vector<Tile>> m_tiles;

            int m_mapWidth;
            int m_mapHeight;

            int m_worldWidth;
            int m_worldHeight;

            int m_backgroundRed;
            int m_backgroundGreen;
            int m_backgroundBlue;
            int m_backgroundAlpha;

            std::string m_activeColourCycle;

            std::list<ParallaxItem> m_parallaxItems;
            std::vector<PaletteCycle> m_paletteCycles;

            std::map<std::string, ParallaxPalette> m_palettes;
            std::map<std::string, ParallaxColour> m_backgroundColours;

            uint32_t m_palette[16 * 4];

            uint8_t *m_bitmap;
            SDL_Texture *m_texture;
            Magick::Pixels *m_tileImageBlob;
            uint8_t *m_tileImageData;

            int m_tileImageWidth;
            int m_tileImageHeight;
    };
}

#endif //NEDRYSOFT_PARALLAXTILEMAP_H
