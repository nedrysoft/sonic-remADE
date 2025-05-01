#
# Copyright (C) 2025 Adrian Carpenter
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# --------------------------------------------------------------------
#
# This file is part of Sonic remADE, a C++ implementation of a
# Sonic The Hedgehog 1 game engine.
#
# https://github.com/nedrysoft/sonic-remade
#

cmake_minimum_required(VERSION 3.21)

project(sonic-remADE)

set(CMAKE_CXX_STANDARD 20)

# if we're on macOS, search for imagemagick pkgconfig file and then add it to the environment variable PKG_CONFIG_PATH so that find_package finds it.

if(APPLE)
    execute_process(COMMAND find /usr/local/Cellar/imagemagick -name "pkgconfig" -print OUTPUT_VARIABLE MAGICK_PREFIX_PATH)

    string(STRIP "${MAGICK_PREFIX_PATH}" MAGICK_PREFIX_PATH)

    set(ENV{PKG_CONFIG_PATH} ${MAGICK_PREFIX_PATH})

    find_package(ImageMagick COMPONENTS Magick++)
endif()

set(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH}
)

set(ENV{ASM_NASM} "C:/Users/Adrian Carpenter/Documents/Development/sonic-remADE/tools/nasm/nasm.exe")

set(SDL2IMAGE_AVIF OFF)
set(SDLMIXER_OPUS OFF)

set(CMAKE_ASM_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT_Embedded "")
set(CMAKE_ASM_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT_ProgramDatabase "")
set(CMAKE_ASM_COMPILE_OPTIONS_MSVC_DEBUG_INFORMATION_FORMAT_EditAndContinue "")

#find_package(PkgConfig REQUIRED)
#find_package(nlohmann_json 3.11.2 REQUIRED) #2.0 REQUIRED)
#find_package(SDL2 REQUIRED COMPONENTS SDL2)
#find_package(SDL2_image 2.0.5 REQUIRED)
#find_package(SDL2_ttf 2.0.18 REQUIRED)
#find_package(SDL2_mixer 2.0.4 REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(SDLTTF_VENDORED ON)
#set(SDL2MIXER_VENDORED ON)
set(SDL2MIXER_VORBIS OFF)
set(SDL2MIXER_OPUS OFF)

add_subdirectory(libs/SDL EXCLUDE_FROM_ALL)
add_subdirectory(libs/SDL_ttf EXCLUDE_FROM_ALL)
add_subdirectory(libs/SDL_mixer EXCLUDE_FROM_ALL)
add_subdirectory(libs/SDL_image EXCLUDE_FROM_ALL)

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(libs/nlohmann_json EXCLUDE_FROM_ALL)

if(APPLE)
    set(LLVM_ENABLE_RTTI ON)
    set(LLVM_ENABLE_EH ON)
endif()

add_executable(sonic-remADE
    src/Animation/Animation.cpp
    src/Animation/AnimationPlayer.cpp
    src/Animation/AnimationSprite.cpp
    src/Animation/AnimationStep.cpp
    src/Animation/Animations.cpp
    src/Audio.cpp
    src/Camera.cpp
    src/DebugManager.cpp
    src/DemoPlayer.cpp
    src/EntryPoint.c
    src/FS.cpp
    src/GameRenderer.cpp
    src/Hud.cpp
    src/Image.cpp
    src/Input.cpp
    src/Main.cpp
    src/NedrysoftIntro.cpp
    src/Object.cpp
    src/ObjectFactory.cpp
    src/ObjectManager.cpp
    src/Objects/Animal.cpp
    src/Objects/BuzzBomber.cpp
    src/Objects/BuzzBomberFactory.cpp
    src/Objects/BuzzBomberProjectile.cpp
    src/Objects/Bridge.cpp
    src/Objects/BridgeFactory.cpp
    src/Objects/Chopper.cpp
    src/Objects/ChopperFactory.cpp
    src/Objects/CollapsingLedge.cpp
    src/Objects/CollapsingLedgeFactory.cpp
    src/Objects/CrabMeat.cpp
    src/Objects/CrabMeatFactory.cpp
    src/Objects/CrabMeatProjectile.cpp
    src/Objects/EdgeWall.cpp
    src/Objects/EdgeWallFactory.cpp
    src/Objects/Explosion.cpp
    src/Objects/GiantRing.cpp
    src/Objects/GiantRingFactory.cpp
    src/Objects/HiddenBonus.cpp
    src/Objects/HiddenBonusFactory.cpp
    src/Objects/Lamppost.cpp
    src/Objects/LamppostFactory.cpp
    src/Objects/Monitor.cpp
    src/Objects/MonitorFactory.cpp
    src/Objects/MotoBug.cpp
    src/Objects/MotoBugFactory.cpp
    src/Objects/Newtron.cpp
    src/Objects/NewtronFactory.cpp
    src/Objects/Platform.cpp
    src/Objects/PlatformFactory.cpp
    src/Objects/Points.cpp
    src/Objects/PurpleRock.cpp
    src/Objects/PurpleRockFactory.cpp
    src/Objects/Rings.cpp
    src/Objects/RingsFactory.cpp
    src/Objects/ScatteredRing.cpp
    src/Objects/Scenery.cpp
    src/Objects/SceneryFactory.cpp
    src/Objects/Signpost.cpp
    src/Objects/SignpostFactory.cpp
    src/Objects/Spikes.cpp
    src/Objects/SpikesFactory.cpp
    src/Objects/Spring.cpp
    src/Objects/SpringFactory.cpp
    src/Objects/Solid.cpp
    src/Objects/Sonic.cpp
    src/Objects/WaterfallSound.cpp
    src/Objects/WaterfallSoundFactory.cpp
    src/Oscillator.cpp
    src/PaletteCycle.cpp
    src/ParallaxTileMap.cpp
    src/SegaIntro.cpp
    src/SonicIntro.cpp
    src/Texture.cpp
    src/Tile.cpp
    src/TileCollision.cpp
    src/TileMap.cpp
    src/Tileset.cpp
    src/TitleCard.cpp
    src/TitleScreen.cpp
    src/Utils.cpp

    include/Animation/Animation.h
    include/Animation/AnimationPlayer.h
    include/Animation/AnimationSprite.h
    include/Animation/AnimationStep.h
    include/Animation/Animations.h
    include/Audio.h
    include/Camera.h
    include/DebugManager.h
    include/DemoPlayer.h
    include/Errors.h
    include/FS.h
    include/GameRenderer.h
    include/Hud.h
    include/Image.h
    include/Input.h
    include/NedrysoftIntro.h
    include/Object.h
    include/ObjectFactory.h
    include/ObjectsManager.h
    include/Objects/Animal.h
    include/Objects/BuzzBomber.h
    include/Objects/BuzzBomberFactory.h
    include/Objects/BuzzBomberProjectile.h
    include/Objects/Bridge.h
    include/Objects/BridgeFactory.h
    include/Objects/Chopper.h
    include/Objects/ChopperFactory.h
    include/Objects/CollapsingLedge.h
    include/Objects/CollapsingLedgeFactory.h
    include/Objects/CrabMeat.h
    include/Objects/CrabMeatFactory.h
    include/Objects/CrabMeatProjectile.h
    include/Objects/EdgeWall.h
    include/Objects/EdgeWallFactory.h
    include/Objects/Explosion.h
    include/Objects/GiantRing.h
    include/Objects/GiantRingFactory.h
    include/Objects/HiddenBonus.h
    include/Objects/HiddenBonusFactory.h
    include/Objects/Lamppost.h
    include/Objects/LamppostFactory.h
    include/Objects/Monitor.h
    include/Objects/MonitorFactory.h
    include/Objects/MotoBug.h
    include/Objects/MotoBugFactory.h
    include/Objects/NewtronFactory.h
    include/Objects/Newtron.h
    include/Objects/Platform.h
    include/Objects/PlatformFactory.h
    include/Objects/Points.h
    include/Objects/PurpleRock.h
    include/Objects/PurpleRockFactory.h
    include/Objects/Rings.h
    include/Objects/RingsFactory.h
    include/Objects/ScatteredRing.h
    include/Objects/Scenery.h
    include/Objects/SceneryFactory.h
    include/Objects/Signpost.h
    include/Objects/SignpostFactory.h
    include/Objects/Spikes.h
    include/Objects/SpikesFactory.h
    include/Objects/Spring.h
    include/Objects/SpringFactory.h
    include/Objects/Solid.h
    include/Objects/Sonic.h
    include/Objects/WaterfallSound.h
    include/Objects/WaterfallSoundFactory.h
    include/Oscillator.h
    include/PaletteCycle.h
    include/ParallaxTileMap.h
    include/SegaIntro.h
    include/SonicIntro.h
    include/Structs.h
    include/TileCollision.h
    include/TileMap.h
    include/Tileset.h
    include/TitleCard.h
    include/TitleScreen.h
    include/Utils.h
)

if (WIN32)
    add_compile_definitions(TARGET_WINDOWS)
elseif(APPLE)
    add_compile_definitions(TARGET_MACOS)
else()
    add_compile_definitions(TARGET_UNIX)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
    sonic-remADE PRIVATE

    SDL2_ttf::SDL2_ttf
    SDL2_mixer::SDL2_mixer
    SDL2_image::SDL2_image
    SDL2::SDL2
    nlohmann_json::nlohmann_json
)

if(APPLE)
    include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${ImageMagick_Magick++_INCLUDE_DIR}
    )

    target_link_libraries(sonic-remADE PRIVATE
        ${ImageMagick_Magick++_LIBRARY}
    )

    target_compile_options(sonic-remADE PRIVATE
        ${PC_Magick++_CFLAGS}
    )
endif()

if(WIN32)
    include_directories(
        "C:/Program Files/ImageMagick-7.1.1-Q8/include"
    )

    target_link_libraries(sonic-remADE PRIVATE
        CORE_RL_Magick++_.lib
        CORE_RL_MagickCore_.lib
    )

    target_link_directories(sonic-remADE PRIVATE
        "C:/Program Files/ImageMagick-7.1.1-Q8/lib"
    )
endif()

target_compile_options(sonic-remADE PRIVATE
    -DNEDRYSOFT_DEBUG

    ${PLATFORM_COMPILE_OPTIONS}
)

target_include_directories(sonic-remADE PRIVATE
    include/Animation
    include/Objects
    include/Libs/3rdParty/Colorize
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

OPTION(DEFINE_MACRO "Option description" ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNEDRYSOFT_DEBUG")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DNEDRYSOFT_DEBUG")
