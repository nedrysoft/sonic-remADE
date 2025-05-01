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

#include "ParallaxTileMap.h"

#include "FS.h"
#include "GameRenderer.h"
#include "Utils.h"

#include <SDL2/SDL.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <nlohmann/json.hpp>

auto constexpr setPixel(uint32_t *pixelBuffer, int x, int y, int width, uint32_t pixel) {
    pixelBuffer[(y *  width) + x] = pixel;
}

Nedrysoft::ParallaxItem::ParallaxItem() :
        m_xOffset(0),
        m_yOffset(0),
        m_startY(0),
        m_endY(0),
        m_frameTicks(0),
        m_autoScrollSpeed(0),
        m_type(ParallaxItemType::Undefined),
        m_textureWidth(0) {

}

Nedrysoft::ParallaxTileMap::ParallaxTileMap() :
        m_mapWidth(0),
        m_mapHeight(0),
        m_worldWidth(0),
        m_worldHeight(0),
        m_backgroundRed(0),
        m_backgroundGreen(0),
        m_backgroundBlue(0),
        m_backgroundAlpha(0),
        m_palette{0} {

    N_UNUSED(m_worldWidth)
    N_UNUSED(m_worldHeight)
}

auto Nedrysoft::ParallaxTileMap::load(const std::string &filename) -> ParallaxTileMap * {
    auto tileMap = new ParallaxTileMap;

    std::ifstream tilemapStream(FS::toNative(filename));

    nlohmann::json jsonObject = nlohmann::json::parse(tilemapStream, nullptr, false);

    if (jsonObject.is_discarded()) {
        delete tileMap;

        std::cout << "error loading json." << std::endl;

        return nullptr;
    }

    try {
        tileMap->m_mapWidth = jsonObject["mapWidth"].get<int>();
        tileMap->m_mapHeight =  jsonObject["mapHeight"].get<int>();

        tileMap->m_tileSet.setTileWidth(jsonObject["tileset"]["tileWidth"].get<int>());
        tileMap->m_tileSet.setTileHeight(jsonObject["tileset"]["tileHeight"].get<int>());

        auto backgroundArray = jsonObject["background"];

        for (auto backgroundObject : backgroundArray) {
            auto name = backgroundObject["name"].get<std::string>();

            ParallaxColour colour = {};

            colour.red = backgroundObject["colour"]["red"].get<int>();
            colour.green = backgroundObject["colour"]["green"].get<int>();
            colour.blue = backgroundObject["colour"]["blue"].get<int>();
            colour.alpha = backgroundObject["colour"]["alpha"].get<int>();

            tileMap->m_backgroundColours[name] = colour;
        }

        /**
         * we default the background colour to "default" if it exists, otherwise the first entry in the map.
         *
         * NOTE: The first entry in the map may not be the first item in the JSON as the map is sorted by key!
         */

        if (!tileMap->m_backgroundColours.empty()) {
            ParallaxColour colour;

            if (tileMap->m_backgroundColours.count("default")) {
                colour = tileMap->m_backgroundColours["default"];
            } else {
                colour = tileMap->m_backgroundColours.begin()->second;
            }

            tileMap->m_backgroundRed = colour.red;
            tileMap->m_backgroundGreen = colour.green;
            tileMap->m_backgroundBlue = colour.blue;
            tileMap->m_backgroundAlpha = colour.alpha;
        }

        auto tilesImage = std::filesystem::path(filename).remove_filename().append(jsonObject["tileset"]["tileSheet"].get<std::string>());

        tileMap->m_tileSet.tileImage().magick("PNG");

        tileMap->m_tileSet.tileImage().read(FS::BLOB(tilesImage.string()));

        auto totalTiles = tileMap->m_mapWidth * tileMap->m_mapHeight;

        tileMap->m_tiles.resize(tileMap->m_mapHeight);

        for (auto y = 0; y < tileMap->m_mapHeight; y++) {
            tileMap->m_tiles[y].resize(tileMap->m_mapWidth);
        }

        for (auto i = 0; i < totalTiles; i++) {
            auto tileX = jsonObject["data"][i]["x"].get<int>();
            auto tileY = jsonObject["data"][i]["y"].get<int>();

            tileMap->m_tiles[tileY][tileX].setTileIndex(jsonObject["data"][i]["tileIndex"].get<int>());
        }

        auto currentY = 0;

        auto paletteArray = jsonObject["palettes"];

        int paletteIndex = 0;

        for (auto paletteObject : paletteArray) {
            auto paletteName = paletteObject["name"].get<std::string>();
            auto palettesArray = paletteObject["data"];

            ParallaxPalette newPalette = {};

            paletteIndex = 0;

            for (std::vector<std::string> palette : palettesArray) {
                for (const std::string &colour : palette) {
                    uint32_t value = hexStringToInt(colour.substr(1, std::string::npos));

                    if ((paletteIndex % 16) != 0) {
                        value |= 0xFF000000;
                    }

                    newPalette.m_data[paletteIndex] = value;

                    paletteIndex++;
                }
            }

            tileMap->m_palettes[paletteName] = newPalette;
        }

        if (!tileMap->m_palettes.empty()) {
            if (tileMap->m_palettes.count("default")) {
                memcpy(tileMap->m_palette, tileMap->m_palettes["default"].m_data, sizeof(tileMap->m_palette));
            } else {
                memcpy(tileMap->m_palette, tileMap->m_palettes.begin()->second.m_data, sizeof(tileMap->m_palette));
            }
        } else {
            memset(tileMap->m_palette, 0, sizeof(tileMap->m_palette));
        }

        auto cycleArray = jsonObject["palette-cycles"];

        for (auto cycle : cycleArray) {
            auto name = cycle["name"].get<std::string>();
            auto frames = cycle["frames"].get<int>();

            std::vector<PaletteCycleData> cycles;

            auto dataArray = cycle["data"];

            for(auto data : dataArray) {
                std::vector<uint32_t> colours;

                auto startIndex = data["start-index"].get<int>();
                auto coloursArray = data["colours"];

                for (std::string color : coloursArray) {
                    colours.push_back(hexStringToInt(color));
                }

                cycles.emplace_back(startIndex, colours);
            }

            tileMap->m_paletteCycles.emplace_back(name, frames, cycles);
        }

        tileMap->m_activeColourCycle = "water";

        auto parallaxArray = jsonObject["parallax"];

        for (auto parallax : parallaxArray) {
            ParallaxItem item;

            auto name = parallax["name"].get<std::string>();
            auto type = parallax["type"].get<std::string>();

            if (type == "sub-blocks") {
                auto height = 0;
                auto subBlockHeight = 1;

                if (parallax.contains("height")) {
                    height = parallax["height"].get<int>();
                }

                if (height == 0) {
                    height = (tileMap->m_mapHeight * tileMap->m_tileSet.tileHeight()) - currentY;
                }

                if (parallax.contains("sub-block-height")) {
                    subBlockHeight = parallax["sub-block-height"].get<int>();

                    if (subBlockHeight < 1) {
                        subBlockHeight = 1;
                    }
                }

                while(height > 0) {
                    item.setName(name);
                    item.setType(ParallaxItemType::Block);
                    item.setStartY(currentY);
                    item.setEndY(currentY + subBlockHeight);
                    item.setAutoScrollSpeed(0.0f);

                    tileMap->m_parallaxItems.push_back(item);

                    currentY += subBlockHeight;

                    height = height - subBlockHeight;
                }
            }

            if (type == "block") {
                auto height = parallax["height"].get<int>();

                if (height == 0) {
                    height = (tileMap->m_mapHeight * tileMap->m_tileSet.tileHeight()) - currentY;
                }

                float autoScrollSpeed = 0.0f;

                if (parallax.contains("auto-scroll-speed")) {
                    autoScrollSpeed = parallax["auto-scroll-speed"].get<float>();
                }

                item.setName(name);
                item.setType(ParallaxItemType::Block);
                item.setStartY(currentY);
                item.setEndY(currentY + height);
                item.setAutoScrollSpeed(autoScrollSpeed);

                tileMap->m_parallaxItems.push_back(item);

                currentY += height;
            }
        }
    } catch (nlohmann::json::exception &e) {
        delete tileMap;

        return nullptr;
    }

    for (auto &cycle : tileMap->m_paletteCycles) {
        if (cycle.name() == tileMap->m_activeColourCycle) {
            cycle.update(reinterpret_cast<uint32_t *>(&tileMap->m_palette));
        }
    }

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    tileMap->m_bitmap = new uint8_t[gameRenderer->viewportWidth() * gameRenderer->viewportHeight()];

    tileMap->m_texture = SDL_CreateTexture(
        gameRenderer->renderer(), 
        SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        gameRenderer->viewportWidth(), 
        gameRenderer->viewportHeight()
    );

    SDL_SetTextureBlendMode(tileMap->m_texture, SDL_BLENDMODE_BLEND);

    tileMap->m_tileImageBlob = new Magick::Pixels(tileMap->m_tileSet.tileImage());
    tileMap->m_tileImageData = reinterpret_cast<uint8_t *>(tileMap->m_tileImageBlob->get(0, 0,  tileMap->m_tileSet.tileImage().size().width(),  tileMap->m_tileSet.tileImage().size().height()));
    tileMap->m_tileImageWidth = tileMap->m_tileSet.tileImage().size().width();
    tileMap->m_tileImageHeight = tileMap->m_tileSet.tileImage().size().height();
    //tileMap->generateTextures();

    return tileMap;
}

auto Nedrysoft::ParallaxTileMap::render(const Vector &position) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();
    
    auto tileWidth = m_tileSet.tileWidth();
    auto tileHeight = m_tileSet.tileHeight();
    
    auto columns = m_tileImageWidth / tileWidth;
    auto rows = m_tileImageHeight / tileHeight;
    
    auto mapPixelsX = m_mapWidth * tileWidth;
    auto mapPixelsY = m_mapHeight * tileHeight;

    auto bytesPerPixel = sizeof(uint32_t);
    auto columnStep = tileWidth * bytesPerPixel;

    auto rowStride = columns * columnStep;
    auto rowStep = rowStride * tileHeight;

    uint32_t *textureData;
    int texturePitch;

    SDL_LockTexture(m_texture, nullptr, reinterpret_cast<void **>(&textureData), &texturePitch);

    auto totalPixels = gameRenderer->viewportWidth() * gameRenderer->viewportHeight();

    float layerOffsets[2] = {
        position.x() * (0.374887079f),
        position.x() * (0.5f)
    };

    uint32_t colour = 0xFFFFFFFF;

    auto waterRowOffset = layerOffsets[1];
    auto waterRowIncrement = (position.x() - layerOffsets[1]) / 104.0f;

    auto itemIterator = m_parallaxItems.begin();

    for (int screenY = 0; screenY < gameRenderer->viewportHeight(); screenY++) {
        int sourceY = position.y();

        while (sourceY + screenY >= itemIterator->endY()) {
            assert(itemIterator != m_parallaxItems.end());

            itemIterator++;
        }

        int layerOffset = static_cast<int>(layerOffsets[0]);

        if (itemIterator->name() == "hills-and-waterfalls") {
            layerOffset = static_cast<int>(layerOffsets[1]);
        } else if (itemIterator->name() == "water") {
            layerOffset =  static_cast<int>(waterRowOffset);
        } else if ( (itemIterator->name() == "upper-clouds") || (itemIterator->name() == "middle-clouds") || (itemIterator->name() == "lower-clouds")) {
            layerOffset =  static_cast<int>(layerOffsets[0] + itemIterator->xOffset());
        }

        for (int screenX = 0; screenX < gameRenderer->viewportWidth(); screenX++) {
            int sourceX = layerOffset;

            auto tileX = (sourceX + screenX) % mapPixelsX;
            auto tileY = (sourceY + screenY) % mapPixelsY;

            auto mapX = tileX / tileWidth;
            auto mapY = tileY / tileHeight;

            int tilePixelX = tileX % tileWidth;
            int tilePixelY = tileY % tileHeight;

            auto tileIndex = m_tiles[mapY][mapX].tileIndex();

            auto tileRow = tileIndex / columns;
            auto tileColumn = tileIndex % columns;

            auto rowPos = (rowStep * tileRow);
            auto columnPos = (columnStep * tileColumn);

            int paletteIndex = m_tileImageData[rowPos + (tilePixelY * rowStride) + columnPos + (tilePixelX * bytesPerPixel) + 3];

            uint32_t colour = m_palette[paletteIndex];

            textureData[(screenY * gameRenderer->viewportWidth()) + screenX]  = colour;
        }
        
        if (itemIterator->name() == "water") {
            waterRowOffset += waterRowIncrement;
        }
    }

    for (auto &parallaxItem : m_parallaxItems) {
        if ( (parallaxItem.name() == "upper-clouds") || (parallaxItem.name() == "middle-clouds") || (parallaxItem.name() == "lower-clouds") ) {
            parallaxItem.adjustOffsets(parallaxItem.autoScrollSpeed(), 0);
        }
    }

    SDL_UnlockTexture(m_texture);

    SDL_Rect rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = gameRenderer->viewportWidth();
    rect.h = gameRenderer->viewportHeight();

    SDL_RenderCopyEx(gameRenderer->renderer(), m_texture, &rect, &rect, 0, nullptr, SDL_FLIP_NONE);
}

auto Nedrysoft::ParallaxTileMap::widthInPixels() const -> int {
    return m_tileSet.tileWidth() * m_mapWidth;
}

auto Nedrysoft::ParallaxTileMap::heightInPixels() const -> int {
    return m_tileSet.tileHeight() * m_mapHeight;
}

auto Nedrysoft::ParallaxTileMap::backgroundRed() const -> uint8_t {
    return m_backgroundRed;
}

auto Nedrysoft::ParallaxTileMap::backgroundGreen() const -> uint8_t {
    return m_backgroundGreen;
}

auto Nedrysoft::ParallaxTileMap::backgroundBlue() const -> uint8_t {
    return m_backgroundBlue;
}

auto Nedrysoft::ParallaxTileMap::backgroundAlpha() const -> uint8_t {
    return m_backgroundAlpha;
}

auto Nedrysoft::ParallaxTileMap::update() -> void {
    for (auto &cycle : m_paletteCycles) {
        if (cycle.name() == m_activeColourCycle) {
            cycle.update(reinterpret_cast<uint32_t *>(&m_palette));
        }
    }
}

auto Nedrysoft::ParallaxTileMap::setPalette(const std::string &name, bool updateTextures) -> void {
    if (m_palettes.count(name)) {
        memcpy(m_palette, m_palettes[name].m_data, sizeof(m_palette));
    }

    if (updateTextures) {
        update();
    }
}

auto Nedrysoft::ParallaxTileMap::setCycle(const std::string &name, bool updateTextures) -> void {
    m_activeColourCycle = name;

    if (updateTextures) {
        update();
    }
}

auto Nedrysoft::ParallaxTileMap::setBackground(const std::string &name, bool updateTextures) -> void {
    if (m_backgroundColours.count(name)) {
        m_backgroundRed = m_backgroundColours[name].red;
        m_backgroundGreen = m_backgroundColours[name].green;
        m_backgroundBlue = m_backgroundColours[name].blue;
        m_backgroundAlpha = m_backgroundColours[name].alpha;
    }

    if (updateTextures) {
        update();
    }
}

auto Nedrysoft::ParallaxTileMap::setColours(const std::string &name) -> void {
    setBackground(name);
    setCycle(name);
    setPalette(name, true);
}

auto Nedrysoft::ParallaxItem::name() -> std::string {
    return m_name;
}

auto Nedrysoft::ParallaxItem::xOffset() const -> float {
    return m_xOffset;
}

auto Nedrysoft::ParallaxItem::yOffset() const -> float {
    return m_yOffset;
}

auto Nedrysoft::ParallaxItem::frameTicks() const -> int {
    return m_frameTicks;
}

auto Nedrysoft::ParallaxItem::startY() const -> int {
    return m_startY;
}

auto Nedrysoft::ParallaxItem::endY() const -> int {
    return m_endY;
}

auto Nedrysoft::ParallaxItem::autoScrollSpeed() const -> float {
    return m_autoScrollSpeed;
}

auto Nedrysoft::ParallaxItem::textureWidth() const -> int {
    return m_textureWidth;
}

auto Nedrysoft::ParallaxItem::textures() -> std::vector<Texture *> & {
    return m_textures;
}

auto Nedrysoft::ParallaxItem::setName(const std::string &name) -> void {
    m_name = name;
}

auto Nedrysoft::ParallaxItem::setXOffset(int offset) -> void {
    m_xOffset = static_cast<float>(offset);
}

auto Nedrysoft::ParallaxItem::setYOffset(int offset) -> void {
    m_yOffset = static_cast<float>(offset);
}

auto Nedrysoft::ParallaxItem::setFrameTicks(int frameTicks) -> void {
    m_frameTicks = frameTicks;
}

auto Nedrysoft::ParallaxItem::setStartY(int y) -> void {
    m_startY = y;
}

auto Nedrysoft::ParallaxItem::setEndY(int y) -> void {
    m_endY = y;
}

auto Nedrysoft::ParallaxItem:: setAutoScrollSpeed(float speed) -> void {
    m_autoScrollSpeed = speed;
}

auto Nedrysoft::ParallaxItem::setTextureWidth(int width) -> void {
    m_textureWidth = width;
}

auto Nedrysoft::ParallaxItem::setType(ParallaxItemType type) -> void {
    m_type = type;
}

auto Nedrysoft::ParallaxItem::type() -> ParallaxItemType {
    return m_type;
}

auto Nedrysoft::ParallaxItem::adjustOffsets(float x, float y) -> void {
    m_xOffset += x;
    m_yOffset += y;
}
