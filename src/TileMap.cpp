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

#include "TileMap.h"

#include "FS.h"
#include "GameRenderer.h"
#include "Utils.h"

#include <SDL2/SDL.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <nlohmann/json.hpp>
#include <set>

/**
 * @brief       The alpha value used to create the solid debug tile overlays.
 */
auto constexpr SolidAlphaLevel = 0x80;

/**
 * @brief       Controls where the the "normal" (vertical) collision data is used to generate collision debug tiles
 *
 * @details     If defined, then the horizontal heights are used to generate the solid debug tiles.  These tiles are
 *              used only when tile debug rendering is enabled to see the terrain, this mode is useful for double
 *              checking that the horizontal (aka rotated) solids match those of the "normal" vertical solids.
 */
//#define DebugUseHorizontalSolids

/**
 * @brief       Sets the colour of a pixel in an image using X & Y coordinates without needing to manually perform
 *              the conversion of the coordinates to an address.
 *
 * @param[in]   pixelBuffer The pixel data.
 * @param[in]   x The x coordinate of the pixel.
 * @param[in]   y The y coordinate of the pixel.
 * @param[in]   width The number of columns in the image.
 * @param[in]   pixel The colour of the pixel in ARGB format.
 */
auto constexpr setPixel(uint32_t *pixelBuffer, int x, int y, int width, uint32_t pixel) {
    pixelBuffer[(y *  width) + x] = pixel;
}

/**
 * @brief       Sets the colour of a pixel using a colour index into a palette.
 *
 * @details     The megadrive hardware uses indexed colours (4 bits per pixel) to draw the tiles, this function
 *              takes a colour index and a palette and sets the pixel by looking up the final colour.
 *
 * @param[in]   pixelBuffer The pixel data.
 * @param[in]   x The x coordinate of the pixel.
 * @param[in]   y The y coordinate of the pixel.
 * @param[in]   width The number of columns in the image.
 * @param[in]   pixel The colour of the pixel in ARGB format.
 */
auto constexpr setIndexedPixel(
        const uint32_t palette[Nedrysoft::PaletteSize],
        uint32_t *pixelBuffer,
        int x,
        int y,
        int width,
        uint32_t pixel) {

    pixelBuffer[(y *  width) + x] = palette[pixel % 64];
}

/**
 * @brief       Returns the colour from a unit colour components to a 32-bit ARGB colour format.
 *
 * @param[in]   r The red component of the colour.
 * @param[in]   g The green component of the colour.
 * @param[in]   b The blue component of the colour.
 * @param[in]   a The alpha component of the colour.
 *
 * @returns     The colour in an uint32_t ARGB format.
 */
constexpr uint32_t pixelARGB(double r, double g, double b, double a) {
    uint32_t value =
            (static_cast<int>(a * 255.0) << 24) |
            (static_cast<int>(b * 255.0) << 0) |
            (static_cast<int>(g * 255.0) << 8) |
            (static_cast<int>(r * 255.0) << 16) ;

    return value;
}

Nedrysoft::TileMap::TileMap() :
        m_mapWidth(0),
        m_mapHeight(0),
        m_visibleTilesInX(0),
        m_visibleTilesInY(0),
        m_lastDebugSolidX(-1),
        m_lastDebugSolidY(-1),
        m_initialPlayerPosition(Vector(0, 0)),
        m_palette{0},
        m_tiles(&m_foregroundMap) {

}

auto Nedrysoft::TileMap::load(const std::string &filename, const std::function<void(TileMap *tileMap, nlohmann::json)> &userLoadFunction) -> TileMap * {
    auto tileMap = new TileMap;

    std::map<std::string, TileFlag> flagMap;

    flagMap["TopSolid"] = TileFlag::TopSolid;
    flagMap["LeftSolid"] = TileFlag::LeftSolid;
    flagMap["RightSolid"] = TileFlag::RightSolid;
    flagMap["BottomSolid"] = TileFlag::BottomSolid;

    flagMap["AllSolid"] = TileFlag::AllSolid;
    flagMap["LeftRightBottomSolid"] = TileFlag::LeftRightBottomSolid;

    flagMap["CalculatedAngle"] = TileFlag::CalculatedAngle;

    flagMap["TopPriority"] = TileFlag::TopPriority;
    flagMap["BottomPriority"] = TileFlag::BottomPriority;
    flagMap["LeftPriority"] = TileFlag::LeftPriority;
    flagMap["RightPriority"] = TileFlag::RightPriority;
    flagMap["TopLeftPriority"] = TileFlag::TopLeftPriority;
    flagMap["TopRightPriority"] = TileFlag::TopRightPriority;
    flagMap["BottomLeftPriority"] = TileFlag::BottomLeftPriority;
    flagMap["BottomRightPriority"] = TileFlag::BottomRightPriority;
    flagMap["AllPriority"] = TileFlag::TopLeftPriority | TileFlag::TopRightPriority | TileFlag::BottomLeftPriority | TileFlag::BottomRightPriority;

    auto tilemapStream = Nedrysoft::FS::IOS(filename);

    auto startTick = SDL_GetTicks64();

    nlohmann::json jsonObject = nlohmann::json::parse(tilemapStream, nullptr, false);

    if (jsonObject.is_discarded()) {
        delete tileMap;

        std::cout << color::rize("[unable to load tilemap]", "Red", "Default", "Bold") +
                             debugValue("filename", filename, "Cyan", "Green", "Bold") << 
                             std::endl;

        return nullptr;
    }

    tileMap->m_initialPlayerPosition = Vector(
        jsonObject["initialPlayerPosition"]["x"].get<float>(),
        jsonObject["initialPlayerPosition"]["y"].get<float>()
    );

    tileMap->m_mapWidth = jsonObject["mapWidth"].get<int>();
    tileMap->m_mapHeight =  jsonObject["mapHeight"].get<int>();

    tileMap->m_tileSet.setTileWidth(jsonObject["tileset"]["tileWidth"].get<int>());
    tileMap->m_tileSet.setTileHeight(jsonObject["tileset"]["tileHeight"].get<int>());

    auto paletteArray = jsonObject["palettes"];

    int paletteIndex = 0;

    for (std::vector<std::string> palette : paletteArray) {
        for (const std::string &colour : palette) {
            uint32_t value = hexStringToInt(colour.substr(1, std::string::npos));

            if ((paletteIndex % ColoursPerPaletteLine) != 0) {
                value |= 0xFF000000;
            }

            tileMap->m_palette[paletteIndex] = value;

            paletteIndex++;
        }
    }

    auto tilesImage = std::filesystem::path(filename).remove_filename().append(jsonObject["tileset"]["tileSheet"].get<std::string>());

    tileMap->m_tileSet.tileImage().magick("PNG");

    tileMap->m_tileSet.tileImage().read(FS::BLOB(tilesImage.string()));

    auto tileSetTotalTiles = jsonObject["tileset"]["totalTiles"].get<int>();

    auto normalArray = jsonObject["tileset"]["collisions"]["normal"];
    auto rotatedArray = jsonObject["tileset"]["collisions"]["rotated"];

    assert(tileSetTotalTiles == normalArray.size());
    assert(tileSetTotalTiles == rotatedArray.size());

    auto tileWidth = tileMap->m_tileSet.tileWidth();
    auto tileHeight = tileMap->m_tileSet.tileHeight();

    assert(tileWidth == tileHeight);

    auto columns = tileMap->m_tileSet.tileImage().size().width() / tileWidth;
    auto rows = tileMap->m_tileSet.tileImage().size().height() / tileHeight;

    tileMap->m_tileSet.collisions()->resize(tileSetTotalTiles);

    for (auto solidIndex = 0; solidIndex < tileSetTotalTiles; solidIndex++) {
        std::vector<int> normalHeights;
        std::vector<int> rotatedHeights;

        normalHeights.resize(tileWidth);
        rotatedHeights.resize(tileHeight);

        for (auto i = 0; i < normalArray[solidIndex].size(); i++) {
            normalHeights[i] = normalArray[solidIndex][i].get<int>();
        }

        for (auto i = 0; i < rotatedArray[solidIndex].size(); i++) {
            rotatedHeights[i] = rotatedArray[solidIndex][i].get<int>();
        }

        (*tileMap->m_tileSet.collisions())[solidIndex] = new TileCollision(normalHeights, rotatedHeights);
    }

    tileMap->m_foregroundMap.resize(tileMap->m_mapHeight);
    tileMap->m_backgroundMap.resize(tileMap->m_mapHeight);

    for (auto y = 0; y < tileMap->m_mapHeight; y++) {
        tileMap->m_foregroundMap[y].resize(tileMap->m_mapWidth);
        tileMap->m_backgroundMap[y].resize(tileMap->m_mapWidth);
    }

    tileMap->createTileImageData(tileSetTotalTiles);

    auto totalTiles = tileMap->m_mapWidth * tileMap->m_mapHeight;

    for (auto i = 0; i < totalTiles; i++) {
        auto tileX = jsonObject["data"][i]["x"].get<int>();
        auto tileY = jsonObject["data"][i]["y"].get<int>();
        auto flags = jsonObject["data"][i]["flags"].get<std::list<std::string>>();
        auto chunkIndex = jsonObject["data"][i]["chunkIndex"].get<int>();
        auto originalTileIndex = jsonObject["data"][i]["originalTileIndex"].get<int>();
        auto originalAngle = jsonObject["data"][i]["originalTileIndex"].get<uint8_t>();
        auto originalSolid = jsonObject["data"][i]["originalTileIndex"].get<uint8_t>();

        Nedrysoft::TileFlag flagValue = Nedrysoft::TileFlag::None;

        for (const auto &flag : flags) {
            if (!flagMap.count(flag)) {
                std::cout << color::rize("[unknown tilemap flag]", "Yellow", "Default", "Bold") +
                     debugValue("flag", flag, "Cyan", "Green", "Bold") << 
                     std::endl;
            } else {
                flagValue |= flagMap[flag];
            }
        }

        tileMap->m_foregroundMap[tileY][tileX].setTileIndex(jsonObject["data"][i]["tileIndex"].get<int>());
        tileMap->m_foregroundMap[tileY][tileX].setAngle(jsonObject["data"][i]["angle"].get<float>());
        tileMap->m_foregroundMap[tileY][tileX].setFlags(flagValue);
        tileMap->m_foregroundMap[tileY][tileX].setChunkIndex(chunkIndex);
        tileMap->m_foregroundMap[tileY][tileX].setOriginalTileIndex(originalTileIndex);
        tileMap->m_foregroundMap[tileY][tileX].setOriginalSolid(originalSolid);
        tileMap->m_foregroundMap[tileY][tileX].setOriginalAngle(originalAngle);


        /**
         * if the top bit of a chunk is set, then it's flagged - in the case of GHZ, this is the loop and the tile
         * entries in this contain a few extra fields that allow us to build the alternative chunk:
         *
         * altFlags         the flags for the background layer, the "loop chunk" has a foreground and a background
         *                  plane, when approaching from the left, sonic is in the foreground plane so is allowed to
         *                  enter the loop, when he progresses to the ceiling, he switches to the background plane
         *                  which allows him to exit the loop (and vice versa is he's going right to left).
         *
         * altAngle         As the collision data is different, there's also an alternative angle.
         * altTileIndex     The tile index to draw, in the case of the GHZ loop visually it looks the same, but we
         *                  have a new tile index due to different collision data.
         * altChunkIndex    The id of the alternative chunk.
         */

        if (chunkIndex & FlaggedChunk) {
            auto altFlags = jsonObject["data"][i]["altFlags"].get<std::list<std::string>>();

            Nedrysoft::TileFlag altFlagValue = Nedrysoft::TileFlag::None;

            for (const auto &flag : altFlags) {
                if (!flagMap.count(flag)) {
                    std::cout << color::rize("[unknown tilemap flag]", "Yellow", "Default", "Bold") +
                                             debugValue("flag", flag, "Cyan", "Green", "Bold") << 
                     std::endl;
                } else {
                    altFlagValue |= flagMap[flag];
                }
            }

            tileMap->m_backgroundMap[tileY][tileX].setTileIndex(jsonObject["data"][i]["altTileIndex"].get<int>());
            tileMap->m_backgroundMap[tileY][tileX].setAngle(jsonObject["data"][i]["altAngle"].get<float>());
            tileMap->m_backgroundMap[tileY][tileX].setFlags(altFlagValue);
            tileMap->m_backgroundMap[tileY][tileX].setChunkIndex(jsonObject["data"][i]["altChunkIndex"].get<int>());
        } else {
            tileMap->m_backgroundMap[tileY][tileX].setTileIndex(jsonObject["data"][i]["tileIndex"].get<int>());
            tileMap->m_backgroundMap[tileY][tileX].setAngle(jsonObject["data"][i]["angle"].get<float>());
            tileMap->m_backgroundMap[tileY][tileX].setFlags(flagValue);
            tileMap->m_backgroundMap[tileY][tileX].setChunkIndex(chunkIndex);
        }
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

    tileMap->m_tileSet.tiles()->resize(tileSetTotalTiles);
    tileMap->m_tileSet.solids()->resize(tileSetTotalTiles);

    std::cout << color::rize("[loaded tilemap]", "Green", "Default", "Bold") +
                             debugValue("filename", filename, "Cyan", "Green", "Bold") << 
                             debugValue("duration", std::to_string(static_cast<float>(SDL_GetTicks64() - startTick) / 1000.0f) + " seconds)", "Cyan", "Green", "Bold") <<
                             std::endl;

    if (userLoadFunction != nullptr) {
        userLoadFunction(tileMap, jsonObject);
    }

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    tileMap->m_visibleTilesInX = ceil((static_cast<float>(gameRenderer->viewportWidth()) / static_cast<float>(tileMap->m_tileSet.tileWidth()) + 0.5) + 1.0);
    tileMap->m_visibleTilesInY = ceil((static_cast<float>(gameRenderer->viewportHeight()) / static_cast<float>(tileMap->m_tileSet.tileHeight()) + 0.5) + 1.0);

    tileMap->generateTextures();

    return tileMap;
}

auto Nedrysoft::TileMap::createTileImageData(int totalTiles) -> void {
    auto image = m_tileSet.tileImage();

    auto tileWidth = m_tileSet.tileWidth();
    auto tileHeight = m_tileSet.tileHeight();

    auto columns = static_cast<int>(image.size().width() / tileWidth);
    auto rows = static_cast<int>(image.size().height() / tileHeight);

    assert(columns > 0);
    assert(rows > 0);

    Magick::Pixels pixelView(image);

    auto pixelData = reinterpret_cast<char *>(pixelView.get(0, 0,  m_tileSet.tileImage().size().width(),  m_tileSet.tileImage().size().height()));

    auto bytesPerTileRow = tileWidth * sizeof(uint32_t);
    auto bytesPerTile = tileHeight * bytesPerTileRow;

    auto columnStep = bytesPerTileRow;
    auto rowStride = columns * columnStep;
    auto rowStep = rowStride * tileHeight;

    for (auto tileIndex = 0; tileIndex < totalTiles; tileIndex++) {

        int y = tileIndex / columns;
        int x = tileIndex % columns;

        auto tilePixels = static_cast<uint32_t *>(malloc(bytesPerTile));
        auto solidPixels = static_cast<uint32_t *>(malloc(bytesPerTile));

        m_tileData[(y * columns) + x] = tilePixels;
        m_solidData[(y * columns) + x] = solidPixels;

        memset(solidPixels, 0x00, bytesPerTile);
        memset(tilePixels, 0x00, bytesPerTile);

        for (auto pixelY = 0; pixelY < tileHeight; pixelY++) {
            for (auto pixelX = 0; pixelX < tileWidth; pixelX++) {
                auto paletteIndex = pixelData[(rowStep * y) + ((pixelY % tileHeight) * rowStride) + (columnStep * x) + ((pixelX % tileWidth) * 4) + 3] & 0xFF;

                setIndexedPixel(
                    m_palette,
                    tilePixels,
                    pixelX,
                    pixelY,
                    tileWidth,
                    paletteIndex
                );
            }
        }

        for (int heightIndex = 0; heightIndex < tileHeight; heightIndex++) {
#if defined(DebugUseHorizontalSolids)
            auto height = (*m_tileSet.collisions())[tileIndex]->horizontalHeight(heightIndex);

            if ( (height == tileWidth) || (height == -tileWidth)) {
                 for (int pixelX = 0; pixelX < tileHeight; pixelX++) {
                    solidPixels[(heightIndex * tileHeight) + pixelX] = 0xFFFFFFFF;
                }
            } else if (height < 0) {
                 for (int pixelX = 0; pixelX < -height; pixelX++) {
                    solidPixels[(heightIndex * tileHeight) + pixelX] = 0xFFFFFFFF;
                }
            } else if (height != 0) {
                for (int pixelX = 0; pixelX < height; pixelX++) {
                    solidPixels[((tileWidth - 1) - pixelX) + (tileWidth * heightIndex)] = 0xFFFFFFFF;
                }
            }
#else
            auto height = (*m_tileSet.collisions())[tileIndex]->verticalHeight(heightIndex);

            if ( (height == tileHeight) || (height == -tileHeight)) {
                 for (int pixelY = 0; pixelY < tileHeight; pixelY++) {
                    solidPixels[(pixelY * tileWidth) + heightIndex] = 0xFFFFFFFF;
                }
            } else if (height < 0) {
                 for (int pixelY = 0; pixelY < -height; pixelY++) {
                    solidPixels[(pixelY * tileWidth) + heightIndex] = 0xFFFFFFFF;
                }
            } else if (height != 0) {
                for (int pixelY = 0; pixelY < height; pixelY++) {
                    solidPixels[(((tileHeight - 1) - pixelY) * tileWidth) + heightIndex] = 0xFFFFFFFF;
                }
            }
        }
#endif
    }
}

auto Nedrysoft::TileMap::generateTextures() -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    void *tileTextureData, *solidTextureData;
    int tileTexturePitch, solidTexturePitch;

    auto startTick = SDL_GetTicks64();

    auto bytesPerTile = m_tileSet.tileWidth() * m_tileSet.tileHeight() * sizeof(uint32_t);

    for (int i = 0; i < m_tileData.size(); i++) {
        auto tile = SDL_CreateTexture(
            gameRenderer->renderer(),
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            m_tileSet.tileWidth(),
            m_tileSet.tileHeight()
        );

        auto solid = SDL_CreateTexture(
            gameRenderer->renderer(),
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            m_tileSet.tileWidth(),
            m_tileSet.tileHeight()
        );

        SDL_LockTexture(tile, nullptr, &tileTextureData, &tileTexturePitch);
        SDL_LockTexture(solid, nullptr, &solidTextureData, &solidTexturePitch);

        memcpy(tileTextureData, m_tileData[i], bytesPerTile);
        memcpy(solidTextureData, m_solidData[i], bytesPerTile);

        SDL_UnlockTexture(tile);
        SDL_UnlockTexture(solid);

        SDL_SetTextureBlendMode(tile, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(solid, SDL_BLENDMODE_BLEND);

        (*m_tileSet.tiles())[i] = new Texture(
            tile,
            m_tileData[i],
            m_tileSet.tileWidth(),
            m_tileSet.tileHeight()
        );

        (*m_tileSet.solids())[i] = new Texture(
            solid,
            m_solidData[i],
            m_tileSet.tileWidth(),
            m_tileSet.tileHeight()
        );
    }

    updateTextures();

    std::cout << color::rize("[generated tilemap textures]", "Green", "Default", "Bold") +
                             debugValue("duration", std::to_string(static_cast<float>(SDL_GetTicks64() - startTick) / 1000.0f) + " seconds)", "Cyan", "Green", "Bold") <<
                             std::endl;
}

auto Nedrysoft::TileMap::initialiseAnimatedBlocks(const std::string &baseFolder, const std::vector<AnimatedBlockState> &loadInfo) -> void {
    std::map<int, std::map<int, std::string>> animatedBlocks;

    for(auto &animatedBlock : loadInfo) {
        for(int block = 0; block < animatedBlock.totalBlocks; block++) {
            for(int frame = 0; frame < animatedBlock.totalFrames; frame++) {
                std::string filename =
                    baseFolder + animatedBlock.baseName + "-" + toIntString(frame + 1) + "-" + toHexString(animatedBlock.blockId + block , 3) + ".png";

                animatedBlocks[animatedBlock.blockId + block][frame] = filename;
            }
        }
    }

    initialiseAnimatedBlocks(animatedBlocks);

    m_animatedBlocksState = loadInfo;
}

auto Nedrysoft::TileMap::initialiseAnimatedBlocks(AnimatedBlockLoadMap &loadMap) -> void {
    auto blockList = std::set<int>();

    for (auto &block : loadMap) {
        blockList.insert(block.first);
    }

    auto tileWidth = m_tileSet.tileWidth();
    auto tileHeight = m_tileSet.tileHeight();
    auto bytesPerPixel = sizeof(uint32_t);
    auto bytesPerTile = tileWidth * tileHeight * bytesPerPixel;

    for (auto blockId : blockList) {
        auto block = reinterpret_cast<AnimatedBlock *>(malloc(sizeof(AnimatedBlock)));

        block->blockId = blockId;
        block->totalFrames = static_cast<int>(loadMap[blockId].size());
        block->frames = reinterpret_cast<uint32_t *>(malloc(bytesPerTile * block->totalFrames));

        auto frameData = block->frames;

        for (auto &info : loadMap[blockId]) {
            Magick::Image image;

            image.magick("PNG");

            image.read(FS::BLOB((info.second)));

            Magick::Pixels pixelData(image);

            int width = static_cast<int>(image.size().width());
            int height = static_cast<int>(image.size().height());

            auto imagePixels =  reinterpret_cast<uint32_t *>(pixelData.get(0, 0,  width,  height));

            // swap raw pixel data from ABGR to RGBA

            for (int pixel = 0; pixel < width * height; pixel++) {
                *(frameData++) = (*(imagePixels) & 0xFF00FF00) | (*(imagePixels) >> 16) & 0x000000FF | (*(imagePixels++) << 16) & 0x00FF0000;
            }
        }

        m_animatedBlocks[blockId] = block;

        m_animatedBlocksFrame[blockId] = 0;
    }
}

auto Nedrysoft::TileMap::setAnimatedBlockFrame(int startingBlockIndex, int frame, int totalBlocks) -> void {
    if (!m_animatedBlocksFrame.count(startingBlockIndex)) {
        return;
    }

    auto tileWidth = m_tileSet.tileWidth();
    auto tileHeight = m_tileSet.tileHeight();
    auto bytesPerPixel = sizeof(uint32_t);
    auto blockIndex = startingBlockIndex;

    do {
        m_animatedBlocksFrame[blockIndex] = frame % m_animatedBlocks[blockIndex]->totalFrames;

        memcpy(reinterpret_cast<void *>((*m_tileSet.tiles())[blockIndex]->data()),
               &m_animatedBlocks[blockIndex]->frames[frame * tileWidth * tileHeight],
               (tileWidth * tileHeight * bytesPerPixel)
        );

        updateTexture(blockIndex++);
    } while(--totalBlocks > 0);
}

auto Nedrysoft::TileMap::updateTexture(int blockIndex) -> void {
    uint32_t *targetPixels;

    int tileTexturePitch;

    if (blockIndex >= m_tileSet.tiles()->size()) {
        return;
    }

    auto tile = (*m_tileSet.tiles())[blockIndex];

    uint32_t *sourcePixels = tile->data();

    SDL_LockTexture(tile->texture(), nullptr, reinterpret_cast<void **>(&targetPixels), &tileTexturePitch);

    for (int y = 0; y < tile->height(); y++) {
        for (int x = 0; x < tile->width(); x++) {
            *(targetPixels++) = *(sourcePixels++);
        }
    }

    SDL_UnlockTexture(tile->texture());
}

auto Nedrysoft::TileMap::updateTextures() -> void {
    uint32_t *tileTextureData;
    int tileTexturePitch;

    int i = 0;

    updateAnimatedBlocks();

    for (auto tile : (*m_tileSet.tiles())) {
        SDL_LockTexture(tile->texture(), nullptr, reinterpret_cast<void **>(&tileTextureData), &tileTexturePitch);

        for (int y = 0; y < tile->height(); y++) {
            for (int x = 0; x < tile->width(); x++) {
                tileTextureData[(y * tile->width()) + x] = tile->data()[(y * tile->width()) + x];
            }
        }

        SDL_UnlockTexture(tile->texture());
    }
}

auto Nedrysoft::TileMap::render(Vector position, RenderFlags renderFlags) -> void {
    int leftTile = MAX(0, position.x() / m_tileSet.tileWidth());
    int rightTile = MIN(m_mapWidth, leftTile + m_visibleTilesInX);
    int topTile = MAX(0, position.y() / m_tileSet.tileHeight());
    int bottomTile = MIN(m_mapHeight, topTile + m_visibleTilesInY);

    int y = -((static_cast<int>(position.y()) % m_tileSet.tileHeight()));

    for(auto tileY = topTile; tileY < bottomTile; tileY++) {
        int x = -((static_cast<int>(position.x()) % m_tileSet.tileWidth()));

        for(auto tileX = leftTile; tileX < rightTile; tileX++) {
            renderTile(m_tileSet.tiles(), tileX, tileY, x, y, renderFlags);

            x += m_tileSet.tileWidth();
        }

        y += m_tileSet.tileHeight();
    }
}

auto Nedrysoft::TileMap::renderTile(std::vector<Texture *> *textures, int tileX, int tileY, int x, int y, RenderFlags renderFlags) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect source, dest;

    if ( (tileY < 0) || (tileY > m_mapHeight - 1) || (tileX < 0) || (tileX > m_mapWidth - 1)) {
        return;
    }

    SDL_Rect fullDestRect;

    fullDestRect.x = x;
    fullDestRect.y = y;
    fullDestRect.w = m_tileSet.tileWidth();
    fullDestRect.h = m_tileSet.tileHeight();

    auto tileIndex = (*m_tiles)[tileY][tileX].tileIndex();

    auto texture = (*textures)[tileIndex]->texture();

    if ((renderFlags & (RenderFlags::SolidOnly | RenderFlags::LastSolidCollisionOnly)) != RenderFlags::All) {
        if (((*m_tiles)[tileY][tileX].flags() & TileFlag::AllSolid) == TileFlag::None) {
            return;
        }

        if ((renderFlags & RenderFlags::LastSolidCollisionOnly) != RenderFlags::All) {
            if ((m_lastDebugSolidX != tileX) || (m_lastDebugSolidY != tileY)) {
                return;
            }
        }

        if (((*m_tiles)[tileY][tileX].flags() & TileFlag::AllSolid) == TileFlag::AllSolid) {
            SDL_SetTextureAlphaMod(texture, SolidAlphaLevel);
            SDL_SetTextureColorMod(texture, 0xFF, 0xFF, 0xFF);
        } else if (((*m_tiles)[tileY][tileX].flags() & TileFlag::AllSolid) == TileFlag::TopSolid) {
            SDL_SetTextureAlphaMod(texture, SolidAlphaLevel);
            SDL_SetTextureColorMod(texture, 0xFF, 0x00, 0x00);
        } else if (((*m_tiles)[tileY][tileX].flags() & TileFlag::AllSolid) == TileFlag::LeftRightBottomSolid) {
            SDL_SetTextureAlphaMod(texture, SolidAlphaLevel);
            SDL_SetTextureColorMod(texture, 0x00, 0x00, 0xFF);
        } else {
            /**
             * we shouldn't get here, but for future reference if I change something, let's set the overlay to green.
             */
            SDL_SetTextureAlphaMod(texture, SolidAlphaLevel);
            SDL_SetTextureColorMod(texture, 0x00, 0xFF, 0x00);
        }

        SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &fullDestRect, 0, nullptr, SDL_FLIP_NONE);

        return;
    }

#if defined(NEDRYSOFT_DEBUG)
    if ( (tileIndex >= 0x190) && (tileIndex <= 0x1B6) ) {
        /**
         * this is a dynamic tile
         */

         switch(tileIndex) {
            case 0x000001ab:            // tall flower - top left
            case 0x000001ac:            // tall flower - top right
            case 0x000001ad:            // tall flower - bottom left
            case 0x000001ae:            // tall flower - bottom right
            case 0x000001af:            // small - top left
            case 0x000001b0:            // small - top right
            case 0x000001b1:            // small - mid left
            case 0x000001b2:            // small - mid right
            case 0x000001b3:            // small - bottom left
            case 0x000001b4: {          // small - bottom right
                break;
            }

            default: {
                SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0x00, 0xFF, 0x80);

                SDL_RenderFillRect(gameRenderer->renderer(), &fullDestRect);

                return;
            }
         }
    }
#endif

    /**
     * this is an early out on the rendering if the render flags have both Priority and Non-Priority selected.
     */

    auto wantedRenderFlags = (RenderFlags::PriorityOnly | RenderFlags::NonPriorityOnly);

    if ((renderFlags & wantedRenderFlags) == wantedRenderFlags) {
        SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &fullDestRect, 0, nullptr, SDL_FLIP_NONE);

        return;
    }

    TileFlag priorityFlags = (*m_tiles)[tileY][tileX].flags() & TileFlag::AllPriority;

    bool renderPriority = ((renderFlags & RenderFlags::PriorityOnly) == (RenderFlags::PriorityOnly));
    bool renderNonPriority = ((renderFlags & RenderFlags::NonPriorityOnly) == (RenderFlags::NonPriorityOnly));

    /**
     * simple early exit for tiles that are either all priority or all non-priority
     */

    if (((priorityFlags & TileFlag::AllPriority) == TileFlag::AllPriority) && renderPriority) {
        SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &fullDestRect, 0, nullptr, SDL_FLIP_NONE);

        return;
    }

    if (((priorityFlags & TileFlag::AllPriority) == TileFlag::None) && renderNonPriority) {
        SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &fullDestRect, 0, nullptr, SDL_FLIP_NONE);

        return;
    }

    /**
     * going for simple here over slightly better optimisation....
     */

    std::list<SDL_Rect> priorityRects;
    std::list<SDL_Rect> noPriorityRects;

    source.x = 0;
    source.y = 0;
    source.w = m_tileSet.tileWidth() / 2;
    source.h = m_tileSet.tileHeight() / 2;

    if ((priorityFlags & TileFlag::TopLeftPriority) == TileFlag::TopLeftPriority) {
        priorityRects.push_back(source);
    } else {
        noPriorityRects.push_back(source);
    }

    source.x = m_tileSet.tileWidth() / 2;
    source.y = 0;
    source.w = m_tileSet.tileWidth() / 2;
    source.h = m_tileSet.tileWidth() / 2;

    if ((priorityFlags & TileFlag::TopRightPriority) == TileFlag::TopRightPriority) {
        priorityRects.push_back(source);
    } else {
        noPriorityRects.push_back(source);
    }

    source.x = 0;
    source.y = m_tileSet.tileHeight() / 2;
    source.w = m_tileSet.tileWidth() / 2;
    source.h = m_tileSet.tileHeight() / 2;

    if ((priorityFlags & TileFlag::BottomLeftPriority) == TileFlag::BottomLeftPriority) {
        priorityRects.push_back(source);
    } else {
        noPriorityRects.push_back(source);
    }

    source.x = m_tileSet.tileWidth() / 2;
    source.y = m_tileSet.tileHeight() / 2;
    source.w = m_tileSet.tileWidth() / 2;
    source.h = m_tileSet.tileHeight() / 2;

    if ((priorityFlags & TileFlag::BottomRightPriority) == TileFlag::BottomRightPriority) {
        priorityRects.push_back(source);
    } else {
        noPriorityRects.push_back(source);
    }

    if (renderPriority) {
        for (auto sourceRect : priorityRects) {
            SDL_Rect destRect;

            destRect = sourceRect;
            destRect.x += x;
            destRect.y += y;

            SDL_RenderCopyEx(gameRenderer->renderer(), texture, &sourceRect, &destRect, 0, nullptr, SDL_FLIP_NONE);
        }
    }

    if (renderNonPriority) {
        for (auto sourceRect : noPriorityRects) {
            SDL_Rect destRect;

            destRect = sourceRect;
            destRect.x += x;
            destRect.y += y;

            SDL_RenderCopyEx(gameRenderer->renderer(), texture, &sourceRect, &destRect, 0, nullptr, SDL_FLIP_NONE);
        }
    }
}

auto Nedrysoft::TileMap::widthInPixels() const -> int {
    return m_tileSet.tileWidth() * m_mapWidth;
}

auto Nedrysoft::TileMap::heightInPixels() const -> int {
    return m_tileSet.tileHeight() * m_mapHeight;
}

auto Nedrysoft::TileMap::initialPlayerPosition() -> Vector {
    return m_initialPlayerPosition;
}

auto Nedrysoft::TileMap::renderDebugOverlay(Vector position) -> void {
    auto leftTile = static_cast<int>(MAX(0, position.x() / m_tileSet.tileWidth()));
    auto rightTile = static_cast<int>(MIN(m_mapWidth, leftTile + m_visibleTilesInX));
    auto topTile = static_cast<int>(MAX(0, position.y() / m_tileSet.tileHeight()));
    auto bottomTile = static_cast<int>(MIN(m_mapHeight, topTile + m_visibleTilesInY));

    auto y = -((static_cast<int>(position.y()) % m_tileSet.tileHeight()));

    for(auto tileY = topTile; tileY < bottomTile; tileY++) {
        auto x = -((static_cast<int>(position.x()) % m_tileSet.tileWidth()));

        for(auto tileX = leftTile; tileX < rightTile; tileX++) {
            renderTile(m_tileSet.solids(), tileX, tileY, x, y, RenderFlags::SolidOnly);

            x += m_tileSet.tileWidth();
        }

        y += m_tileSet.tileHeight();
    }
}

auto Nedrysoft::TileMap::findDown(Vector position, float *distance, float *angle) -> bool {
    auto posX = static_cast<int>(position.x());
    auto posY = static_cast<int>(position.y());

    auto tileWidth = m_tileSet.tileWidth();
    auto tileHeight = m_tileSet.tileHeight();

    auto tileX = posX / tileWidth;
    auto tileY = posY / tileHeight;

    if (tileY > m_mapHeight - 1) {
        return false;
    }

    /*
     * check for a tile at the sensors position.
     */

    if (((*m_tiles)[tileY][tileX].flags() & TileFlag::TopSolid) == TileFlag::None) {
        if (tileY < m_mapHeight - 1) {
            if (((*m_tiles)[tileY + 1][tileX].flags() & TileFlag::TopSolid) == TileFlag::None) {
                *distance = static_cast<float>(((tileHeight - 1) - (posY % tileHeight)) + tileHeight);

                return false;
            }

            tileY++;
        } else {
            *distance = static_cast<float>((m_mapHeight * tileHeight) - posY);

            return false;
        }
    } else {
        auto height = abs((*m_tileSet.collisions())[(*m_tiles)[tileY][tileX].tileIndex()]->verticalHeight(posX % tileWidth));

        if (height == tileHeight) {
            if (tileY > 0) {
                auto regressionTile = (*m_tiles)[tileY - 1][tileX];

                if ( (regressionTile.flags() & TileFlag::TopSolid) != TileFlag::None) {
                    if ((*m_tileSet.collisions())[regressionTile.tileIndex()]->verticalHeight(posX % tileWidth) != 0) {
                        tileY--;
                    }
                }
            }
        } else if (height == 0) {
            if (tileY < m_mapHeight - 1) {
                auto extensionTile = (*m_tiles)[tileY + 1][tileX];

                if ( (extensionTile.flags() & TileFlag::TopSolid) != TileFlag::None) {
                    if ((*m_tileSet.collisions())[extensionTile.tileIndex()]->verticalHeight(posX % tileWidth) != 0) {
                        tileY++;
                    }
                }
            } else {
                *distance = static_cast<float>((m_mapHeight * tileHeight) - posY);

                return false;
            }
        }
    }

    /*
     * if we've got here, we've found a tile that has the solid surface, so now we can figure out how far from that
     * surface we are.
     */

    auto startingY = 0;
    auto endingY = 0;

    auto tile = (*m_tiles)[tileY][tileX];

    auto height = (*m_tileSet.collisions())[tile.tileIndex()]->verticalHeight(posX % tileWidth);

    if (height < 0) {
        startingY = (tileY * tileHeight);
        endingY = (tileY * tileHeight) - height;
    } else {
        startingY = (tileY * tileHeight) + tileHeight - height;
        endingY = (tileY * tileHeight) + tileHeight;
    }

    if (posY <= endingY) {
        *distance = static_cast<float>(startingY - posY);
    } else {
        *distance = static_cast<float>(endingY - posY);
    }

    if (angle) {
        if ((tile.flags() & TileFlag::CalculatedAngle) != TileFlag::None) {
            *angle = -1;
        } else {
            *angle = static_cast<float>(tile.angle());
        }
    }

    return true;
}

auto Nedrysoft::TileMap::findRight(Vector position, float *distance, float *angle) -> bool {
    auto posX = static_cast<int>(position.x());
    auto posY = static_cast<int>(position.y());

    auto tileWidth = m_tileSet.tileWidth();
    auto tileHeight = m_tileSet.tileHeight();

    auto tileX = posX / tileWidth;
    auto tileY = posY / tileHeight;

    /*
     * check for a tile at the sensors position.
     */

    if (((*m_tiles)[tileY][tileX].flags() & TileFlag::LeftRightBottomSolid) == TileFlag::None) {
        if (tileX < m_mapWidth - 1) {
            if (((*m_tiles)[tileY][tileX + 1].flags() & TileFlag::LeftRightBottomSolid) == TileFlag::None) {
                *distance = static_cast<float>(((tileWidth - 1) - (posX % tileWidth)) + tileWidth);

                return false;
            }

            tileX++;
        } else {
            *distance = static_cast<float>((m_mapWidth * tileWidth) - posX);

            return false;
        }
    } else {
        auto height = abs((*m_tileSet.collisions())[(*m_tiles)[tileY][tileX].tileIndex()]->horizontalHeight(posY % tileHeight));

        if (height == 0) {
            if (tileX < m_mapWidth - 1) {
                auto extensionTile = (*m_tiles)[tileY][tileX + 1];

                if ( (extensionTile.flags() & TileFlag::LeftRightBottomSolid) != TileFlag::None) {
                    if ((*m_tileSet.collisions())[extensionTile.tileIndex()]->horizontalHeight(posY % tileHeight) != 0) {
                        tileX++;
                    }
                }
            } else {
                *distance = static_cast<float>((m_mapWidth * tileHeight) - posX);

                return false;
            }
        } else if (height == tileWidth) {
            if (tileX > 0) {
                auto regressionTile = (*m_tiles)[tileY][tileX - 1];

                if ( (regressionTile.flags() & TileFlag::LeftRightBottomSolid) != TileFlag::None) {
                    if ((*m_tileSet.collisions())[regressionTile.tileIndex()]->horizontalHeight(posY % tileHeight) != 0) {
                        tileX--;
                    }
                }
            }
        }
    }

    /*
     * if we've got here, we've found a tile that has the solid surface, so now we can figure out how far from that
     * surface we are.
     */

    int startingX, endingX;

    auto tile = (*m_tiles)[tileY][tileX];

    auto height = (*m_tileSet.collisions())[tile.tileIndex()]->horizontalHeight(posY % tileHeight);

    if (height < 0) {
        startingX = (tileX * tileWidth);
        endingX = (tileX * tileWidth) - height;
    } else {
        startingX = (tileX * tileWidth) + tileWidth - height;
        endingX = (tileX * tileWidth) + tileWidth;
    }

    if (posX <= endingX) {
        *distance = static_cast<float>(startingX - posX);
    } else {
        *distance = static_cast<float>(endingX - posX);
    }

    if (angle) {
        if ((tile.flags() & TileFlag::CalculatedAngle) != TileFlag::None) {
            *angle = -1;
        } else {
            *angle = static_cast<float>(tile.angle());
        }
    }

    return true;
}

auto Nedrysoft::TileMap::findLeft(Vector position, float *distance, float *angle) -> bool {
    auto posX = static_cast<int>(position.x());
    auto posY = static_cast<int>(position.y());

    auto tileWidth = m_tileSet.tileWidth();
    auto tileHeight = m_tileSet.tileHeight();

    auto tileX = posX / tileWidth;
    auto tileY = posY / tileHeight;

    /*
     * check for a tile at the sensors position.
     */

    if (((*m_tiles)[tileY][tileX].flags() & TileFlag::LeftRightBottomSolid) == TileFlag::None) {
        if (tileX > 0) {
            if (((*m_tiles)[tileY][tileX - 1].flags() & TileFlag::LeftRightBottomSolid) == TileFlag::None) {
                *distance = static_cast<float>(((tileWidth - 1) - (posX % tileWidth)) + tileWidth);

                return false;
            }

            tileX--;
        } else {
            *distance = static_cast<float>((m_mapWidth * tileWidth) - posX);

            return false;
        }
    } else {
        auto height = abs((*m_tileSet.collisions())[(*m_tiles)[tileY][tileX].tileIndex()]->horizontalHeight(posY % tileHeight));

        if (height == 0) {
            if (tileX > 0) {
                auto extensionTile = (*m_tiles)[tileY][tileX - 1];

                if ( (extensionTile.flags() & TileFlag::LeftRightBottomSolid) != TileFlag::None) {
                    if ((*m_tileSet.collisions())[extensionTile.tileIndex()]->horizontalHeight(posY % tileHeight) != 0) {
                        tileX--;
                    }
                }
            } else {
                *distance = static_cast<float>((m_mapWidth * tileWidth) - posX);

                return false;
            }
        } else if (height == tileWidth) {
            if (tileX > m_mapWidth - 1) {
                auto regressionTile = (*m_tiles)[tileY][tileX + 1];

                if ( (regressionTile.flags() & TileFlag::LeftRightBottomSolid) != TileFlag::None) {
                    if ((*m_tileSet.collisions())[regressionTile.tileIndex()]->horizontalHeight(posY % tileHeight) != 0) {
                        tileX++;
                    }
                }
            }
        }
    }

    /*
     * if we've got here, we've found a tile that has the solid surface, so now we can figure out how far from that
     * surface we are.
     */

    int startingX, endingX;

    auto tile = (*m_tiles)[tileY][tileX];

    auto height = (*m_tileSet.collisions())[tile.tileIndex()]->horizontalHeight(posY % tileHeight);

    if (height < 0) {
        startingX = (tileX * tileWidth);
        endingX = (tileX * tileWidth) - height;
    } else {
        startingX = (tileX * tileWidth) + tileWidth - height;
        endingX = (tileX * tileWidth) + tileWidth;
    }

    if (posX >= startingX) {
        *distance = static_cast<float>(posX - endingX);
    } else {
        *distance = static_cast<float>(startingX - posX);
    }

    if (angle) {
        if ((tile.flags() & TileFlag::CalculatedAngle) != TileFlag::None) {
            *angle = -1;
        } else {
            *angle = static_cast<float>(tile.angle());
        }
    }

    return true;
}

auto Nedrysoft::TileMap::findUp(Vector position, float *distance, float *angle) -> bool {
    auto posX = static_cast<int>(position.x());
    auto posY = static_cast<int>(position.y());

    auto tileWidth = m_tileSet.tileWidth();
    auto tileHeight = m_tileSet.tileHeight();

    auto tileX = posX / tileWidth;
    auto tileY = posY / tileHeight;

    /*
     * check for a tile at the sensors position.
     */

    if (((*m_tiles)[tileY][tileX].flags() & TileFlag::LeftRightBottomSolid) == TileFlag::None) {
        if (tileY > 0) {
            if (((*m_tiles)[tileY - 1][tileX].flags() & TileFlag::LeftRightBottomSolid) == TileFlag::None) {
                *distance = static_cast<float>(((tileHeight - 1) - (posY % tileHeight)) + tileHeight);

                return false;
            }

            tileY--;
        } else {
            *distance = static_cast<float>((m_mapHeight * tileHeight) - posY);

            return false;
        }
    } else {
        auto height = abs((*m_tileSet.collisions())[(*m_tiles)[tileY][tileX].tileIndex()]->verticalHeight(posX % tileWidth));

        if (height == tileHeight) {
            if (tileY < m_mapHeight - 1) {
                auto regressionTile = (*m_tiles)[tileY + 1][tileX];

                if ( (regressionTile.flags() & TileFlag::LeftRightBottomSolid) != TileFlag::None) {
                    if ((*m_tileSet.collisions())[regressionTile.tileIndex()]->verticalHeight(posX % tileWidth) != 0) {
                        tileY++;
                    }
                }
            }
        } else if (height == 0) {
            if (tileY > 0) {
                auto extensionTile = (*m_tiles)[tileY - 1][tileX];

                if ( (extensionTile.flags() & TileFlag::LeftRightBottomSolid) != TileFlag::None) {
                    if ((*m_tileSet.collisions())[extensionTile.tileIndex()]->verticalHeight(posX % tileWidth) != 0) {
                        tileY--;
                    }
                }
            } else {
                *distance = static_cast<float>((m_mapHeight * tileHeight) - posY);

                return false;
            }
        }
    }

    /*
     * if we've got here, we've found a tile that has the solid surface, so now we can figure out how far from that
     * surface we are.
     */

    int startingY, endingY;

    auto tile = (*m_tiles)[tileY][tileX];

    auto height = (*m_tileSet.collisions())[tile.tileIndex()]->verticalHeight(posX % tileWidth);

    if (height < 0) {
        startingY = (tileY * tileHeight);
        endingY = (tileY * tileHeight) - height;
    } else {
        startingY = (tileY * tileHeight) + tileHeight - height;
        endingY = (tileY * tileHeight) + tileHeight;
    }

    if (posY >= startingY) {
        *distance = static_cast<float>(posY - endingY);
    } else {
        *distance = static_cast<float>(startingY - posY);
    }

    if (angle) {
        if ((tile.flags() & TileFlag::CalculatedAngle) != TileFlag::None) {
            *angle = -1;
        } else {
            *angle = static_cast<float>(tile.angle());
        }
    }

    return true;
}

auto Nedrysoft::TileMap::update() -> void {
    for (auto &block : m_animatedBlocksState) {
        if ((block.frameTimer--) == 0) {
            block.currentFrame = (block.currentFrame + 1) % static_cast<int>(block.sequence.size());
            block.frameTimer = block.sequence[block.currentFrame].duration;

            setAnimatedBlockFrame(block.blockId, block.sequence[block.currentFrame].frame, block.totalBlocks);
        }
    }

    bool regenerateTextures = false;

    for (auto &cycle : m_paletteCycles) {
        regenerateTextures = regenerateTextures | cycle.update(reinterpret_cast<uint32_t *>(&m_palette));
    }

    if (!regenerateTextures) {
        return;
    }
}

auto Nedrysoft::TileMap::updateAnimatedBlocks() -> void {
    auto bytesPerTile = m_tileSet.tileWidth() * m_tileSet.tileHeight() * static_cast<int>(sizeof(uint32_t));

    for (auto block : m_animatedBlocks) {
        auto targetPixelData = (*m_tileSet.tiles())[block.first]->data();

        int offset = m_animatedBlocksFrame[block.first] * bytesPerTile;

        memcpy(reinterpret_cast<void *>(targetPixelData), &block.second->frames[offset], bytesPerTile);

        updateTexture(block.first);
    }
}

auto Nedrysoft::TileMap::chunkIndex(int x, int y) -> int {
    return (*m_tiles)[y / m_tileSet.tileHeight()][x / m_tileSet.tileWidth()].chunkIndex();
}

auto Nedrysoft::TileMap::chunkIndex(float x, float y) -> int {
    y = static_cast<int>(y) / m_tileSet.tileHeight();
    x = static_cast<int>(x) / m_tileSet.tileWidth();

    if ( (x > m_mapWidth - 1) || (y > m_mapHeight - 1) || (x < 0) || (y < 0) ) {
        return -1;
    }

    return (*m_tiles)[y][x].chunkIndex();
}

auto Nedrysoft::TileMap::setActivePlane(TileMapPlane plane) -> void {
    if (plane == TileMapPlane::Foreground) {
        m_tiles = &m_foregroundMap;
    } else {
        m_tiles = &m_backgroundMap;
    }
}
