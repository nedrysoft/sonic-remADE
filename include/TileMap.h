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

#ifndef NEDRYSOFT_TILEMAP_H
#define NEDRYSOFT_TILEMAP_H

#include "PaletteCycle.h"
#include "Structs.h"
#include "Texture.h"
#include "Tile.h"
#include "Tileset.h"

#include <Magick++.h>
#include <SDL2/SDL_render.h>
#include <functional>
#include <nlohmann/json.hpp>
#include <string>

namespace Nedrysoft {
    constexpr auto ColoursPerPaletteLine = 16;
    constexpr auto TotalPaletteLines = 4;
    constexpr auto PaletteSize = ColoursPerPaletteLine * TotalPaletteLines;
    constexpr auto FlaggedChunk = 0x80;

    /**
     * @brief       Determines what tiles should be rendered
     *
     * @details     These values are used in arguments to render() calls and determine exactly which types of
     *              tile should be rendered during the call.
     */
    enum class RenderFlags {
        All = 0,                                        //<<! All tiles will be rendered.

        PriorityOnly = 1,                               //<<! Only tiles with the priority flag set will be rendered.
        NonPriorityOnly = 2,                            //<<! Only tiles with the priority flag clear will be rendered.
        SolidOnly = 4,                                //<<! Only tiles that have solidity will be rendered.
        LastSolidCollisionOnly = 8                          //<<! Only the tile that was the last collision target will be rendered.
    };

    inline constexpr RenderFlags operator & (RenderFlags x, RenderFlags y) {
        return static_cast<RenderFlags>(static_cast<int>(x) & static_cast<int>(y));
    }

    inline constexpr RenderFlags operator | (RenderFlags x, RenderFlags y) {
        return static_cast<RenderFlags>(static_cast<int>(x) | static_cast<int>(y));
    }

    /**
     * @brief       A structure that defines an animated tilemap block.
     */
    struct AnimatedBlock {
        int blockId;                                    //<! The block id used in the tile map.
        int totalFrames;                                //<! The total number of frames in this animation.
        uint32_t *frames;                               //<! The pixel data for all the frames of animation in this block.
    };

    /**
     * @brief       A structure that determines the animation of a set of background tiles.
     *
     * @details     Each block of tiles requires a "script" that the tilemap engine can use to update the animation,
     *              and in the case of the background tile animations, we keep need to know which frame to show and
     *              the duration that it should remain on that frame before advancing to the next.
     *
     * @note        Animations may include a frame more than once.
     */
    struct AnimatedBlockStep {
        int frame;                                      //<! The frame number that is to be displayed.
        int duration;                                   //<! The duration in frames that this steps frame should be displayed for.
    };

    /**
     * @brief       A structure used to maintain the current state of animated tiles.
     *
     * @details     The foreground tilemap contains a number of tiles that are dynamically swapped out for
     *              different versions, in the Green Hill Zone, the flowers are swapped out to create an animation
     *              that brings an amount of life to what is otherwise a static background.
     *
     *              This structure contains the information and data about how the animation progresses frame by
     *              frame and the state is updated to run the animation.
     */
    struct AnimatedBlockState {
        int blockId;                                    //<! The starting block id of the tiles to swap out.
        int totalBlocks;                                //<! The total number of blocks to swap out.
        int totalFrames;                                //<! The total number of frames in the animation.
        int currentFrame;                               //<! The current frame being displayed for this animation.
        int frameTimer;                                 //<! The timer that updates the animation when it reaches zero.
        std::string baseName;                           //<! The base name of the sprites to be used when loading the frames.
        std::vector<AnimatedBlockStep> sequence;        //<! The animation sequence, a list of timings that trigger the animation.
    };

    /**
     * @brief       A structure for creating the dynamic tiles required for chunks with bit 7 set.
     *
     * @details     The original sonic engine creates a dual plane tilemap so that it can deal with the loop
     *              objects in the game, the loop chunk is mirrored in the alternative plane so that once Sonic
     *              has passed 180 degrees, the tilemap is swapped out for the alternative one which contains
     *              the mirrored chunk that allows Sonic to complete the loop.
     *
     *              This engine does not use the 256x265 chunks or the 8x8 tiles for the tile map, instead it
     *              deals only with the 16x16 blocks, so we use this structure to hold the found "flagged chunks"
     *              so that we can create the second plane at load time.
     *
     *              The index of the original tile is stored inside the structure, these extra tiles are discovered while
     *              parsing the map during load and these tiles are dynamically added to the tileset, because the tiles
     *              are only loop tiles, both the position of the tile in ins chunk and the collision data needs mirrored
     *              to generate the correct information for the second half of the loop.
     */
    struct ExtraTile {
        int x;                                          //<<! The x position of the tile in the map.
        int y;                                          //<<! The y position of the tile in the map.
        int tileIndex;                                  //<<! The index of the tile originally at the coordinates.
        int newTileIndex;                               //<<! The new image of the tile to be used at the coordinates.
    };

    /**
     * @brief       Determines which tile map plane is currently active
     *
     * @details     The TileMap class represents the foreground terrain that Sonic interacts with, but there is a
     *              further sub-level within this to deal with the in game loops.  As the loops require no collision
     *              on entry or exit, it's not possible to define a loop with a single "chunk", so the original
     *              megadrive game resolves this by creating a "foreground plane" and a "background plane" where
     *              the only difference is the entrance/exit of the loop.
     *
     *              The game logic determines which plane should be active according to Sonics current state and
     *              position.
     */
    enum class TileMapPlane {
        Foreground,                                     //<<! The plane is in the foreground.
        Background                                      //<<! The plane is in the background.
    };

    /**
     * @brief       A structure to encapsulate the data for loading an animated tile map sprite
     *
     * @details     The tilemap is almost entirely static, but the flowers in the Green Hill Zone background have a
     *              small amount of animation.  On the megadrive this is accomplished directly using the tilemap
     *              and simply updating the tiles causing every instance in the map to render with the current
     *              frame of the animation.
     *
     *              This double map structure defines the order of sprites and frames and allows the engine to load
     *              these extra animated tiles in the correct order for rendering and for their animation.
     */
    typedef std::map<int, std::map<int, std::string>> AnimatedBlockLoadMap;

    /**
     * @brief       The foreground tilemap implementation
     *
     * @details     Renders the foreground terrain data, it should be noted that the megadrive VDP chip has the
     *              ability to allow sprites to appear over or under this layer using the priority bit, this
     *              implementation includes this functionality with a dual rendering process.
     *
     *              The level data is stored in a JSON document and it contains details such as the level layout,
     *              the players starting position, palette cycling information, collision data and so forth.
     *
     *              While this class notionally is used to represent the foreground, the original sonic game included
     *              loops which require extra handling to ensure that the player does not end up running into an
     *              invisible wall, as a result of this this foreground tilemap has two planes, the "foreground" and
     *              the "background" planes.
     *
     *              The background plane should not be confused with the background tilemap, instead the background
     *              plane is an almost identical copy of the foreground plane barring the loop chunk which contains
     *              the mirrored collision data, when the player hits the loop, the engine will dynamically switch
     *              between the foreground and background planes so that the player can traverse the loops correctly.
     */
    class TileMap {
        public:
            /**
             * @brief       Loads and parses a JSON tilemap.
             *
             * @details     The file is loaded and parsed and a TileMap instance is returned.  A user provided load function
             *              allows the caller to parse and extract any extra information that may be present in the
             *              JSON file.
             *
             * @param[in]   filename tile file to load.
             * @param[in]   userLoadFunction a user supplied function for additional custom parsing or nullptr if not required.
             *
             * @returns     The loaded tile map if it was successfully loaded, otherwise nullptr.
             */
            static auto load(
                const std::string &filename,
                const std::function<void(class TileMap *tileMap, nlohmann::json)> &userLoadFunction = nullptr
            ) -> TileMap *;

            /**
             * @brief       Returns the players starting position.
             *
             * @returns     A vector that contains the pixel coordinates of the players bottom left point.
             */
            auto initialPlayerPosition() -> Vector;

            /**
             * @brief       Updates the map
             *
             * @details     The map must be updated each frame, each call to update will advance any frame based
             *              features.  The animated flowers in the background have counters that are decremented
             *              each frame, once the counter has elapsed this will trigger updating of tiles to their
             *              next state.
             */
            auto update() -> void;

            /**
             * @brief       Renders the map
             *
             * @details     The tilemap may be rendered more than once per frame so that any sprite or tile priority
             *              behaviour is replicated from the original Sonic game.
             *
             * @param[in]   position The position of the viewport, the map is rendered relative to this coordinate.
             * @param[in]   renderFlags The flags that determine exactly what is rendered per call.
             */
            auto render(Vector position, RenderFlags renderFlags = RenderFlags::All) -> void;

            /**
             * @brief       Renders the debug overlay
             *
             * @details     The debug overlay should always be rendered after all other render calls to the tilemap
             *              have been completed.  The debug overlay includes information that aid debugging or
             *              visualisation of the engine.
             *
             * @param[in]   position The position of the viewport, the map is rendered relative to this coordinate.
             */
            auto renderDebugOverlay(Vector position) -> void;

            /**
             * @brief       Registers animated blocks with the tilemap.
             *
             * @details     The animated blocks are registered with the tilemap with this function.
             *
             * @param[in]   baseFolder The location where the images for the animated blocks can be found.
             * @param[in]   loadInfo The list of blocks that are to be loaded and initialised.
             */
            auto initialiseAnimatedBlocks(const std::string &baseFolder, const std::vector<AnimatedBlockState> &loadInfo) -> void;

            /**
             * @brief       Sets which plane is currently active.
             *
             * @details     While this class notionally is used to represent the foreground, the original sonic game included
             *              loops which require extra handling to ensure that the player does not end up running into an
             *              invisible wall, as a result of this this foreground tilemap has two planes, the "foreground" and
             *              the "background" planes.
             *
             *              The background plane should not be confused with the background tilemap, instead the background
             *              plane is an almost identical copy of the foreground plane barring the loop chunk which contains
             *              the mirrored collision data, when the player hits the loop, the engine will dynamically switch
             *              between the foreground and background planes so that the player can traverse the loops correctly.
             *
             * @param[in]   plane The plane to be selected.
             */
            auto setActivePlane(TileMapPlane plane) -> void;

            /**
             * @brief       Finds the block closest to the given coordinate using a downward facing sensor ray.
             *
             * @details     The collision detection is implemented by taking a vector and firing a ray outwards from it
             *              until it hits a solid surface or it has passed the maximum distance that we are looking for
             *              the surface.
             *
             *              The original megadrive game was limited by the power of the hardware, so the collision detection
             *              will only search for a solid surface within 2 tiles (including the tile in which the vector
             *              resides in).
             *
             *              The function will return the distance to the surface if one was found, if no surface was found
             *              then the maximum distance travelled by the ray will be returned, you should be aware that in
             *              this case the distance is not constant and will change according to the plays Y position
             *              within a block as it cannot tell where the surface is because it only looks for a maximum of
             *              2 tiles away.
             *
             * @param[in]   position The coordinate of the sensor.
             * @param[out]  distance The distance to the surface or the maximum distance the ray travelled.
             * @param[out]  angle The angle of the surface if the ray hit a solid surface.
             *
             * @returns     True if the ray hit a surface, otherwise false.
             */
            auto findDown(Vector position, float *distance, float *angle = nullptr) -> bool;

            /**
             * @brief       Finds the block closest to the given coordinate using a right facing sensor ray.
             *
             * @details     The collision detection is implemented by taking a vector and firing a ray outwards from it
             *              until it hits a solid surface or it has passed the maximum distance that we are looking for
             *              the surface.
             *
             *              The original megadrive game was limited by the power of the hardware, so the collision detection
             *              will only search for a solid surface within 2 tiles (including the tile in which the vector
             *              resides in).
             *
             *              The function will return the distance to the surface if one was found, if no surface was found
             *              then the maximum distance travelled by the ray will be returned, you should be aware that in
             *              this case the distance is not constant and will change according to the plays Y position
             *              within a block as it cannot tell where the surface is because it only looks for a maximum of
             *              2 tiles away.
             *
             * @param[in]   position The coordinate of the sensor.
             * @param[out]  distance The distance to the surface or the maximum distance the ray travelled.
             * @param[out]  angle The angle of the surface if the ray hit a solid surface.
             *
             * @returns     True if the ray hit a surface, otherwise false.
             */
            auto findRight(Vector position, float *distance, float *angle = nullptr) -> bool;

            /**
             * @brief       Finds the block closest to the given coordinate using a left facing sensor ray.
             *
             * @details     The collision detection is implemented by taking a vector and firing a ray outwards from it
             *              until it hits a solid surface or it has passed the maximum distance that we are looking for
             *              the surface.
             *
             *              The original megadrive game was limited by the power of the hardware, so the collision detection
             *              will only search for a solid surface within 2 tiles (including the tile in which the vector
             *              resides in).
             *
             *              The function will return the distance to the surface if one was found, if no surface was found
             *              then the maximum distance travelled by the ray will be returned, you should be aware that in
             *              this case the distance is not constant and will change according to the plays Y position
             *              within a block as it cannot tell where the surface is because it only looks for a maximum of
             *              2 tiles away.
             *
             * @param[in]   position The coordinate of the sensor.
             * @param[out]  distance The distance to the surface or the maximum distance the ray travelled.
             * @param[out]  angle The angle of the surface if the ray hit a solid surface.
             *
             * @returns     True if the ray hit a surface, otherwise false.
             */
            auto findLeft(Vector position, float *distance, float *angle = nullptr) -> bool;

            /**
             * @brief       Finds the block closest to the given coordinate using a upward facing sensor ray.
             *
             * @details     The collision detection is implemented by taking a vector and firing a ray outwards from it
             *              until it hits a solid surface or it has passed the maximum distance that we are looking for
             *              the surface.
             *
             *              The original megadrive game was limited by the power of the hardware, so the collision detection
             *              will only search for a solid surface within 2 tiles (including the tile in which the vector
             *              resides in).
             *
             *              The function will return the distance to the surface if one was found, if no surface was found
             *              then the maximum distance travelled by the ray will be returned, you should be aware that in
             *              this case the distance is not constant and will change according to the plays Y position
             *              within a block as it cannot tell where the surface is because it only looks for a maximum of
             *              2 tiles away.
             *
             * @param[in]   position The coordinate of the sensor.
             * @param[out]  distance The distance to the surface or the maximum distance the ray travelled.
             * @param[out]  angle The angle of the surface if the ray hit a solid surface.
             *
             * @returns     True if the ray hit a surface, otherwise false.
             */
            auto findUp(Vector position, float *distance, float *angle = nullptr) -> bool;

            /**
             * @brief       Gets the original chunk index (as per the original megadrive game) from a vector.
             *
             * @overload    auto TileMap::chunkIndex(int x, int y) -> int
             *
             * @param[in]   x The vectors X position.
             * @param[in]   y The vectors Y position.
             *
             * @returns     The original chunk index that was used in the original game.
             */
            auto chunkIndex(int x, int y) -> int;

            /**
             * @brief       Gets the original chunk index (as per the original megadrive game) from a vector.
             *
             * @note        The floating point coordinates will be cast to an int without any rounding.
             *
             * @overload    auto TileMap::chunkIndex(float x, float y) -> int
             *
             * @param[in]   x The vectors X position.
             * @param[in]   y The vectors Y position.
             *
             * @returns     The original chunk index that was used in the original game.
             */
            auto chunkIndex(float x, float y) -> int;

            /**
             * @brief       Gets the width of the tilemap in pixels.
             *
             * @returns     The width in pixels.
             */
            [[nodiscard]] auto widthInPixels() const -> int;

            /**
             * @brief       Gets the height of the tilemap in pixels.
             *
             * @returns     The height in pixels.
             */
            [[nodiscard]] auto heightInPixels() const -> int;

        private:
            /**
             * @brief       Constructs a new TileMap
             *
             * @details     Instances of a TileMap cannot be created directly, they are always crated by using the
             *              static TileMap::load function which will load the tilemap and return a pointer to the
             *              newly crated instance of the TileMap class.
             */
            TileMap();

            /**
             * @brief       Destroys the TileMap.
             */
            ~TileMap() = default;

        private:

            /**
             * @brief       Renders a single tile in the tilemap.
             *
             * @param[in]   textures A vector containing all the SDL textures for each tile used in the map.
             * @param[in]   tileX The X coordinate in the map of the tile to be rendered.
             * @param[in]   tileY The Y coordinate in the map of the tile to be rendered.
             * @param[in]   x The X position in screen coordinates of where the tile should be drawn.
             * @param[in]   y The Y position in screen coordinates of where the tile should be drawn.
             * @param[in]   renderFlags The flags to be used to determine if and how the tile should be rendered.
             */
            auto renderTile(std::vector<Texture *> *textures, int tileX, int tileY, int x, int y, RenderFlags renderFlags = RenderFlags::All) -> void;

            /**
             * @brief       Converts the raw pixel data for the tiles into textures that SDL can render directly.
             */
            auto generateTextures() -> void;

            /**
             * @brief       Regenerates all the SDL textures from the source data.
             */
            auto updateTextures() -> void;

            /**
             * @brief       Regenerates the SDL texture for a specific block.
             *
             * @param[in]   blockIndex The block index to regenerate the texture for.
             */
            auto updateTexture(int blockIndex) -> void;

            /**
             * @brief       Creates raw pixel data from the tilemap PNG.
             *
             * @details     Magick++ is used to load the tileset image, the image is then split into the blocks
             *              that the engine will actually use.  The Tilemap uses a "faux" indexed colour system where
             *              only one channel of the image is used to store the original palette index that was in
             *              the original game, as the tilemap has the palette information in, this function will
             *              convert the "faux" indexed colour into the correct colours for rendering.
             *
             * @param[in]   totalTiles The total number of tiles to generate data for.
             */
            auto createTileImageData(int totalTiles) -> void;

            /**
             * @brief       Loads and initialises the tilemap animated tiles.
             *
             * @details     Nearly every single tile in the tilemap remains the same throughout the level, however,
             *              there are some tiles that are dynamically animated during the game loop, in the Green
             *              Hills Zone the flowers that form part of the terrain are animated using these extra
             *              animated blocks.
             *
             * @param[in]   loadMap A map that contains a list of animated tiles that are to be used in the level.
             */
            auto initialiseAnimatedBlocks(AnimatedBlockLoadMap &loadMap) -> void;

            /**
             * @brief       Updates the animation state of a range of animated blocks.
             *
             * @details     The animated parts of the background consist of multiple blocks that are need to be updated,
             *              by supplying the starting block index and the total number of blocks to update, each
             *              sub tile of the animated background can be advanced through the animation by updating its
             *              current frame.
             *
             * @param[in]   startingBlockIndex The first block index to update.
             * @param[in]   frame The new frame index to be displayed.
             * @param[in]   totalBlocks The total number of blocks to be updated.
             */
            auto setAnimatedBlockFrame(int startingBlockIndex, int frame, int totalBlocks = 0) -> void;

            /**
             * @brief       Updates the tile SDL textures for the animated tiles.
             *
             * @details     After advancing an animated tiles frame, the SDL textures will need to be updated to reflect
             *              the new frame pixel data, this function will update all of the animated blocks to ensure
             *              that the textures are in sync with the actual tile data.
             */
            auto updateAnimatedBlocks() -> void;

        private:
            Tileset m_tileSet;                                      //<<! The tileset used by this tilemap.

            std::map<int, uint32_t *> m_tileData;                   //<<! The raw pixel data by the tile renderer.
            std::map<int, uint32_t *> m_solidData;                  //<<! The raw pixel data used solid debug renderer.

            std::vector<std::vector<Tile>> *m_tiles;                //<<! The current tile data for the active "plane".
            std::vector<std::vector<Tile>> m_foregroundMap;         //<<! The foreground tile map.
            std::vector<std::vector<Tile>> m_backgroundMap;         //<<! The background tile map.

            int m_mapWidth;                                         //<<! The width of the tilemap in tiles (from the tileset).
            int m_mapHeight;                                        //<<! The height of the tilemap in tiles (from the tileset).

            int m_visibleTilesInX;                                  //<<! The maximum number of tiles visible horizontally in the viewport.
            int m_visibleTilesInY;                                  //<<! The maximum number of tiles visible vertically in the viewport.

            int m_lastDebugSolidX;                                  //<<! Holds the tiles X position of the last collision.
            int m_lastDebugSolidY;                                  //<<! Holds the tiles Y position of the last collision.

            Vector m_initialPlayerPosition;                         //<<! The starting position in the level (in pixels).

            uint32_t m_palette[ColoursPerPaletteLine * TotalPaletteLines];  //<<! The current palette used to render the tilemap.

            std::map<int, AnimatedBlock *> m_animatedBlocks;        //<<! The list of animated blocks used in the tilemap.
            std::vector<AnimatedBlockState> m_animatedBlocksState;  //<<! The data that maintains the animated blocks.
            std::vector<PaletteCycle> m_paletteCycles;              //<<! Data that defines how the palette should be cycled.

            std::map<int, int> m_animatedBlocksFrame;
    };
}

#endif //NEDRYSOFT_TILEMAP_H