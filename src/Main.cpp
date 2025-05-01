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

#include "Game.h"

#include "Audio.h"
#include "Camera.h"
#include "DebugManager.h"
#include "DemoPlayer.h"
#include "Errors.h"
#include "FS.h"
#include "GameRenderer.h"
#include "Hud.h"
#include "Input.h"
#include "NedrysoftIntro.h"
#include "ObjectsManager.h"
#include "Oscillator.h"
#include "ParallaxTileMap.h"
#include "SegaIntro.h"
#include "Sonic.h"
#include "SonicIntro.h"
#include "TileMap.h"
#include "TitleCard.h"
#include "TitleScreen.h"
#include <MagickCore/MagickCore.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_render.h>
#include <SDL_ttf.h>
#include <iostream>

#include <variant>

#ifndef TARGET_WINDOWS
#include <unistd.h>
#endif

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
#pragma ide diagnostic ignored "Simplify"

#define SoftwareRenderer
//#define DisableDemo

//#define DisableAudio                                                //!< disables both sound and music (overrides DisableSound & DisableMusic)

//#define DisableSound                                                //!< disables just sounds
//#define DisableMusic                                                //!< disables just music

//#define DisableIntro
//#define DisableTitleCard
//#define DisableTitleScreen

//#define DebugManagerEnabled

auto constexpr ViewportWidth = 320;
auto constexpr ViewportHeight = 224;

auto constexpr ViewportScale = 4;

auto constexpr CameraDebug = "cameraDebug";
auto constexpr SolidsDebug = "solidsDebug";
auto constexpr ForegroundTileMap = "foregroundTileMap";
auto constexpr BackgroundTileMap = "backgroundTileMap";
auto constexpr PlayerDebug = "playerDebug";
auto constexpr ObjectsCollisionDebug = "objectsCollisionDebug";
auto constexpr SensorRays = "sensorRays";
auto constexpr StartDemo = "startDemo";

static int frameCount = 0;

extern "C" int gameEntrypoint(int argc, char **argv);

int gameEntrypoint(int argc, char **argv) {
    Magick::InitializeMagick(*argv);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
		std::cout << "Fatal Error: Couldn't initialize SDL. (" << SDL_GetError() << ")" << std::endl;

		return Nedrysoft::ErrorCodes::SDLInitialisationFailed;
	}

#if !defined(DisableAudio)
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        std::cout << "Error: Couldn't initialize SDL Mixer. (" << SDL_GetError() << ")" << std::endl;
    }
#endif

    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    Mix_AllocateChannels(2);

    //SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "0");

    SDL_Rect windowRenderRect;

    windowRenderRect.x = 0;
    windowRenderRect.y = 0;

    windowRenderRect.w = ViewportWidth * ViewportScale;
    windowRenderRect.h = ViewportHeight * ViewportScale;

    auto window = SDL_CreateWindow(
	    "Sonic remADE",
	    SDL_WINDOWPOS_UNDEFINED,
	    SDL_WINDOWPOS_UNDEFINED,
	    windowRenderRect.w,
        windowRenderRect.h,
	    0//SDL_WINDOW_FULLSCREEN
	);

    if (!window) {
        std::cout << "Fatal Error: Couldn't create SDL window.  (" << SDL_GetError() << ")" << std::endl;

        return Nedrysoft::ErrorCodes::SDLCreateWindowFailed;
    }

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    auto windowSurface = SDL_GetWindowSurface(window);

    if (!windowSurface) {
        std::cout << "Fatal Error: Couldn't get window surface. (" << SDL_GetError() << ")" << std::endl;

        return Nedrysoft::ErrorCodes::SDLGetWindowSurfaceFailed;
    }

#ifndef SoftwareRenderer
#error There's no support for a non-software renderer!
#endif

    /**
     * we use a software renderer because using any kind of hardware rendering absolutely drives the performance
     * to a halt as we need to modify the textures a lot because the megadrive uses colour cycling to create
     * animations, and as a result, we need to update the textures a lot - which if they're stored in the video
     * RAM of the graphics card makes them a very slow operation.
     *
     * As we're not making use or doing anything 3D, we can flip to the software renderer which makes everything
     * run super fast.
     */

    auto renderer = SDL_CreateSoftwareRenderer(windowSurface);

    if (!renderer) {
        std::cout << "Fatal Error: Couldn't create software renderer. (" << SDL_GetError() << ")" << std::endl;

        return Nedrysoft::ErrorCodes::SDLCreateSoftwareRendererFailed;
    }

    SDL_Rect gameRenderRect;

    gameRenderRect.x = 0;
    gameRenderRect.y = 0;
    gameRenderRect.w = ViewportWidth;
    gameRenderRect.h = ViewportHeight;

    auto renderTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, gameRenderRect.w, gameRenderRect.h);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_ShowCursor(0);

    auto font = TTF_OpenFontRW(Nedrysoft::FS::SDL("./data/fonts/Born2bSportyV2.ttf"), true, 16);

    if (!font) {
        std::cout << "Fatal Error:  Unable to load font. (" << SDL_GetError() << ")" << std::endl;

        return Nedrysoft::ErrorCodes::SDLLoadingFontFailed;
    }

    auto audio = Nedrysoft::Audio::getInstance();

    audio->initialise();

    /**
     * we initialise an instance of GameRenderer as soon as we have finished initialising SDL, as GameRenderer
     * is a singleton, we can then access it to whenever needed from this point on without having to pass it in
     * to functions as a parameter.
     */

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    gameRenderer->initialise(window, renderer, ViewportWidth, ViewportHeight, ViewportScale);
    gameRenderer->setDefaultFont(font);

    /**
     * The DebugManager class provides a basic system to toggling debug information on and off.  Macros are provided
     * that further wrap up the class, allowing the use of "if <debug option>" tests, in debug builds the conditional
     * will query the DebugManger class to determine whether the feature is on or off.  In a release build the
     * macro will resolve to "if (true)" or "if (false)" depending on the macro chosen, so no further if statements
     * are required around test to ensure the code gets compiled correctly.
     */

#if defined(DebugManagerEnabled)
    auto debugManager = Nedrysoft::DebugManager::getInstance();

    assert(debugManager != nullptr);

    debugManager->registerToggle(CameraDebug, "Camera Debug", SDL_SCANCODE_C, false);
    debugManager->registerToggle(SolidsDebug, "Solids Debug", SDL_SCANCODE_S, false);
    debugManager->registerToggle(ForegroundTileMap, "Foreground", SDL_SCANCODE_F, true);
    debugManager->registerToggle(BackgroundTileMap, "Background", SDL_SCANCODE_B, true);
    debugManager->registerToggle(PlayerDebug, "Player Debug", SDL_SCANCODE_P, false);
    debugManager->registerToggle(ObjectsCollisionDebug, "Object Collisions", SDL_SCANCODE_O, false);
    debugManager->registerToggle(StartDemo, "Start Demo", SDL_SCANCODE_D, false);
#endif

    Nedrysoft::DemoPlayer demoPlayer;

    demoPlayer.load("data/ghz-1.json");

    /**
     * this would load the demo data from the ghz-1.json file, that file would also load the correct foreground tilemap,
     * background tilemap and objects data as well, so for the moment, this doesn't happen as I haven't yet added
     * a "level" class that encapsulates all this data.
     */

    std::list<Nedrysoft::Camera::Limit> cameraLimits;

    std::vector<Nedrysoft::AnimatedBlockState> blockList;

    auto tileMap = Nedrysoft::TileMap::load(Nedrysoft::FS::toNative("./data/ghz-1-foreground-tilemap.json"), [&](Nedrysoft::TileMap *tileMap, nlohmann::json jsonObject) {
        if (!jsonObject.contains("animatedBlocks")) {
            return;
        }

        for (auto block : jsonObject["animatedBlocks"]) {
            Nedrysoft::AnimatedBlockState blockInfo;

            blockInfo.blockId = block["block"].get<int>();
            blockInfo.totalBlocks = block["blocks"].get<int>();
            blockInfo.totalFrames = block["frames"].get<int>();
            blockInfo.baseName = block["basename"].get<std::string>();

            for (auto step : block["sequence"]) {
                Nedrysoft::AnimatedBlockStep blockStep{};

                blockStep.frame = step["frame"].get<int>();
                blockStep.duration = step["duration"].get<int>();

                blockInfo.sequence.push_back(blockStep);
            }

            blockInfo.currentFrame = blockInfo.sequence[0].frame;
            blockInfo.frameTimer = blockInfo.sequence[0].duration;

            blockList.push_back(blockInfo);
        }

        tileMap->initialiseAnimatedBlocks(Nedrysoft::FS::toNative("./data/art/objects/ghz-misc/"), blockList);

        Nedrysoft::Camera::Limit *previousLimit = nullptr;

        if (jsonObject.contains("cameraLimits")) {
            for (auto limit : jsonObject["cameraLimits"]) {
                Nedrysoft::Camera::Limit newLimit = {};

                if (previousLimit) {
                    newLimit.startX = previousLimit->endX;
                } else {
                    newLimit.startX = 0;
                }

                if (limit.contains("x")) {
                    newLimit.endX = limit["x"].get<float>();
                } else {
                    newLimit.endX = static_cast<float>(tileMap->widthInPixels());
                }

                newLimit.y = limit["y"].get<float>();

                cameraLimits.push_back(newLimit);

                previousLimit = &cameraLimits.back();
            }
        }
    });

    if (!tileMap) {
        return Nedrysoft::ErrorCodes::LoadingTileMapFailed;
    }

    auto backgroundTileMap = Nedrysoft::ParallaxTileMap::load(Nedrysoft::FS::toNative("./data/ghz-1-background-tilemap.json"));

    if (!backgroundTileMap) {
        return Nedrysoft::ErrorCodes::LoadingBackgroundTileMapFailed;
    }

    auto camera = new Nedrysoft::Camera;

    assert(camera != nullptr);

    camera->setWorldSize(tileMap->widthInPixels(), tileMap->heightInPixels());
    camera->setLimits(cameraLimits);

    auto sonic = Nedrysoft::Sonic::getInstance();

    assert(sonic != nullptr);

    sonic->setInitialPosition(tileMap->initialPlayerPosition());

    camera->setInitialPosition(sonic->position());

    auto objectsManager = Nedrysoft::ObjectsManager::getInstance();

    if (!objectsManager->load(Nedrysoft::FS::toNative(("./data/ghz-1-objects.json")))) {
        return Nedrysoft::ErrorCodes::LoadingObjectsFailed;
    }

#if defined(DebugManagerEnabled)
    auto frameStepButtonPressed = false;
    auto frameAdvance = false;
    auto frameStepEnabled = false;
#endif

    auto hud = Nedrysoft::Hud::getInstance();

    audio->start();

    std::list<float> fpsData;

#if !defined(DisableIntro)
    SDL_RenderSetScale(renderer, ViewportScale, ViewportScale);

    Nedrysoft::NedrysoftIntro::execute(window, renderer, ViewportWidth, ViewportHeight);

    Nedrysoft::SegaIntro::execute(window, renderer, ViewportWidth, ViewportHeight);
    Nedrysoft::SonicIntro::execute(window, renderer, ViewportWidth, ViewportHeight);

    SDL_RenderSetScale(renderer, 1, 1);
#endif

#if !defined(DisableTitleScreen)
    Nedrysoft::TitleScreen titleScreen;

    titleScreen.start();

    float scrollPosition = 0;

    backgroundTileMap->setColours("title");

    Nedrysoft::Audio::getInstance()->playMusic(Nedrysoft::MusicId::Title);

    while(handleEvents()) {
        gameRenderer->startFrameTimer();

        gameRenderer->beginRendering();

        SDL_SetRenderTarget(renderer, renderTexture);

        SDL_SetRenderDrawColor(
            renderer,
            backgroundTileMap->backgroundRed(),
            backgroundTileMap->backgroundGreen(),
            backgroundTileMap->backgroundBlue(),
            backgroundTileMap->backgroundAlpha()
        );

        SDL_RenderClear(renderer);

        Nedrysoft::Vector backgroundPosition = {
            scrollPosition,
            0,
        };

        backgroundTileMap->render(backgroundPosition);

        titleScreen.render();

        SDL_SetRenderTarget(renderer, nullptr);

        SDL_RenderCopy(renderer, renderTexture, &gameRenderRect, &windowRenderRect);

        SDL_UpdateWindowSurface(window);

        gameRenderer->endRendering();

        if (titleScreen.finished()) {
            backgroundTileMap->update();

            scrollPosition += 2;
        }

        if (Nedrysoft::Input::getInstance()->pressed(Nedrysoft::JoystickButton::Start)) {
            break;
        }

        gameRenderer->endFrameTimer();
        gameRenderer->waitForNextFrame();
    }


    if (!Nedrysoft::Input::getInstance()->pressed(Nedrysoft::JoystickButton::Start)) {
        return 0;
    }

    titleScreen.end();

    while(handleEvents()) {
        gameRenderer->beginRendering();

        SDL_SetRenderTarget(renderer, renderTexture);

        SDL_SetRenderDrawColor(
            renderer,
            backgroundTileMap->backgroundRed(),
            backgroundTileMap->backgroundGreen(),
            backgroundTileMap->backgroundBlue(),
            backgroundTileMap->backgroundAlpha()
        );

        SDL_RenderClear(renderer);

        Nedrysoft::Vector backgroundPosition = {
            scrollPosition,
            0,
        };

        backgroundTileMap->render(backgroundPosition);

        titleScreen.render();

        SDL_SetRenderTarget(renderer, nullptr);

        SDL_RenderCopy(renderer, renderTexture, &gameRenderRect, &windowRenderRect);

        SDL_UpdateWindowSurface(window);

        gameRenderer->endRendering();

        backgroundTileMap->update();

        scrollPosition += 2;

        if (titleScreen.finished()) {
            break;
        }
    }
#endif

#if !defined(DisableTitleCard)
    Nedrysoft::TitleCard titleCard;

    titleCard.start(Nedrysoft::GreenHill, Nedrysoft::One);
#endif

    backgroundTileMap->setColours("default");

#if !defined(DisableMusic)
    Nedrysoft::Audio::getInstance()->playMusic(Nedrysoft::MusicId::GHZ1);
#endif

#if defined(DisableAudio)
    Nedrysoft::Audio::getInstance()->disableAudio();
#endif

    hud->reset();

    Nedrysoft::Oscillator *oscillator = Nedrysoft::Oscillator::getInstance();

    bool restarting = false;
    int alpha = -1;

    while(handleEvents()) {
        gameRenderer->startFrameTimer();
        //auto frameStart = std::chrono::high_resolution_clock::now();

        gameRenderer->beginRendering();

        frameCount++;

        /**
         * we render to a texture that is the same size at the original megadrive, we have to do this rather than
         * use the built in SDL scaling because otherwise the performance is terrible.  By rendering directly to a texture
         * which is sized to the original size, everything is a 1:1 mapping and we simply then copy this texture to the
         * window surface, incurring just a single scaling operation, everything then works incredibly fast.
         */

        SDL_SetRenderTarget(renderer, renderTexture);

        /**
         * clear the viewport with the background colour retrieved from the background tile map
         */

        SDL_SetRenderDrawColor(
            renderer,
            backgroundTileMap->backgroundRed(),
            backgroundTileMap->backgroundGreen(),
            backgroundTileMap->backgroundBlue(),
            backgroundTileMap->backgroundAlpha()
        );

        SDL_RenderClear(renderer);

#if defined(DebugManagerEnabled)
        if (Nedrysoft::Input::getInstance()->pressed(Nedrysoft::JoystickButton::B)) {
            static bool first = true;

            if (first) {
                debugManager->toggle(SDL_SCANCODE_P);

                first = false;
            }

            frameStepButtonPressed = true;
            frameStepEnabled = true;
        } else {
            if (frameStepButtonPressed) {
                frameStepButtonPressed = false;
                frameAdvance = true;
            } else {
                frameAdvance = false;
            }
        }

        if ((!frameStepEnabled) || ((frameStepEnabled) && (frameAdvance))) {
#endif
            sonic->update(tileMap, camera);

            if (sonic->isDead()) {
                if (!restarting) {
                    restarting = true;
                    alpha = 0x00;
                } else {
                    if (alpha >= 255) {
                        hud->reset();

                        sonic->addLives(-1);
                        
                        sonic->setInitialPosition(tileMap->initialPlayerPosition());

                        camera->setInitialPosition(sonic->position());

                        backgroundTileMap->update();

#if !defined(DisableTitleCard)
                        titleCard.start(Nedrysoft::GreenHill, Nedrysoft::One);
#endif
                        objectsManager->reset();

                        restarting = false;
                    }

                }

            } 

            oscillator->update(sonic);

            camera->update(sonic);

            if ((!sonic->isDying()) && (!sonic->isDead())) {
                objectsManager->update(tileMap, camera, sonic);
            }

            backgroundTileMap->update();

            tileMap->update();

            if (DebugManager_IsOn(StartDemo, true)) {
                demoPlayer.update();
            }

#if defined(DebugManagerEnabled)
            frameAdvance = false;
        }
#endif

        if (DebugManager_IsOn(BackgroundTileMap, true)) {
            Nedrysoft::Vector backgroundPosition = {
                camera->position().x(),
                std::max(0.0f, 35.0f - (camera->position().y() / 36.0f))
            };

            backgroundTileMap->render(backgroundPosition);
        }

        if (DebugManager_IsOn(ForegroundTileMap, true)) {
            tileMap->render(camera->position(), Nedrysoft::RenderFlags::NonPriorityOnly);
        }

        objectsManager->render(camera, Nedrysoft::ObjectRenderPriority::NonPriorityOnly);

        sonic->render(camera);

        if (DebugManager_IsOn(CameraDebug, false)) {
            camera->renderDebugOverlay();
        }

        if (DebugManager_IsOn(ObjectsCollisionDebug, false)) {
            objectsManager->renderCollisions(camera);
        }

        if (DebugManager_IsOn(PlayerDebug, false)) {
            sonic->renderDebug(camera);
        }

        if (DebugManager_IsOn(ForegroundTileMap, true)) {
            tileMap->render(camera->position(), Nedrysoft::RenderFlags::PriorityOnly);
        }

        if (DebugManager_IsOn(SolidsDebug, false)) {
            tileMap->renderDebugOverlay(camera->position());
        }

        objectsManager->render(camera, Nedrysoft::ObjectRenderPriority::PriorityOnly);

        hud->render(sonic);

#if !defined(DisableTitleCard)
        titleCard.render();
#endif

        if (restarting) {
            SDL_Rect dest;

            dest.x = 0;
            dest.y = 0;
            dest.w = gameRenderer->viewportWidth();
            dest.h = gameRenderer->viewportHeight();

            SDL_SetRenderDrawColor(gameRenderer->renderer(), 0x00, 0x00, 0x00, alpha);

            SDL_RenderFillRect(gameRenderer->renderer(), &dest);

            SDL_SetRenderTarget(renderer, nullptr);

            SDL_RenderCopy(renderer, renderTexture, &gameRenderRect, &windowRenderRect);

            SDL_UpdateWindowSurface(window);

            alpha = std::max(0, alpha + 24);
        }

#if defined(NEDRYSOFT_DEBUG)
        debugManager->render();
#endif

        /**
         * once we're done rendering, we restore the render target back to the default and then blit from the
         * texture we were rendering into, into the windows texture.  This is unnecessarily convoluted but
         * has to be done this way otherwise the performance is horrible when scaling.
         */

        SDL_SetRenderTarget(renderer, nullptr);

        SDL_RenderCopy(renderer, renderTexture, &gameRenderRect, &windowRenderRect);

        SDL_UpdateWindowSurface(window);

        gameRenderer->endRendering();

        if ((frameCount % 60) == 0) {
            auto fpsString = std::to_string(static_cast<int>(round(gameRenderer->averageFramesPerSecond()))) + " FPS";

            Nedrysoft::DebugManager::getInstance()->addText("fps", fpsString, {0x00, 0xFF, 0x00, 0xFF});

            auto ticksString = std::to_string(static_cast<int>(round(Nedrysoft::DebugManager::getInstance()->ticks("frame")))) + " ms";

            Nedrysoft::DebugManager::getInstance()->addText("ticks", ticksString, {0x00, 0xFF, 0x00, 0xFF});
        }

        gameRenderer->endFrameTimer();
        gameRenderer->waitForNextFrame();
/*
#if defined(SoftwareRenderer)
        auto frameTime = std::chrono::high_resolution_clock::now() - frameStart;

        double frameMicroseconds = std::chrono::duration_cast<std::chrono::microseconds>(frameTime).count();

        if (frameMicroseconds <= MicrosecondsPerFrame) {
            usleep(MicrosecondsPerFrame - frameMicroseconds);
        }
#endif*/
	}

    Mix_Quit();

	IMG_Quit();

	SDL_DestroyRenderer(renderer);

	SDL_DestroyWindow(window);

	SDL_Quit();

    return 0;
}

auto handleEvents() -> bool {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                return false;
            }

            case SDL_JOYDEVICEADDED:
            case SDL_JOYDEVICEREMOVED:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP: {
                Nedrysoft::Input::getInstance()->handleEvent(event);
                break;
            }

            case SDL_KEYDOWN: {
                if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                    event.type = SDL_JOYBUTTONDOWN;
                    event.jbutton.button = static_cast<uint8_t>(Nedrysoft::JoystickButton::Left);
                    event.jbutton.state = SDL_PRESSED;

                    Nedrysoft::Input::getInstance()->handleEvent(event);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                    event.type = SDL_JOYBUTTONDOWN;
                    event.jbutton.button = static_cast<uint8_t>(Nedrysoft::JoystickButton::Right);
                    event.jbutton.state = SDL_PRESSED;
                    
                    Nedrysoft::Input::getInstance()->handleEvent(event);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    event.type = SDL_JOYBUTTONDOWN;
                    event.jbutton.button = static_cast<uint8_t>(Nedrysoft::JoystickButton::A);
                    event.jbutton.state = SDL_PRESSED;
                    
                    Nedrysoft::Input::getInstance()->handleEvent(event);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    event.type = SDL_JOYBUTTONDOWN;
                    event.jbutton.button = static_cast<uint8_t>(Nedrysoft::JoystickButton::Start);
                    event.jbutton.state = SDL_PRESSED;

                    Nedrysoft::Input::getInstance()->handleEvent(event);
                }

                break;
            }

            case SDL_KEYUP: {
                auto debugManager = Nedrysoft::DebugManager::getInstance();

                debugManager->toggle(event.key.keysym.scancode);

                if (event.key.keysym.scancode == SDL_SCANCODE_Q) {
                    exit(0);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                    event.type = SDL_JOYBUTTONUP;
                    event.jbutton.button = static_cast<uint8_t>(Nedrysoft::JoystickButton::Left);
                    event.jbutton.state = SDL_RELEASED;

                    Nedrysoft::Input::getInstance()->handleEvent(event);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                    event.type = SDL_JOYBUTTONUP;
                    event.jbutton.button = static_cast<uint8_t>(Nedrysoft::JoystickButton::Right);
                    event.jbutton.state = SDL_RELEASED;
                    
                    Nedrysoft::Input::getInstance()->handleEvent(event);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    event.type = SDL_JOYBUTTONUP;
                    event.jbutton.button = static_cast<uint8_t>(Nedrysoft::JoystickButton::A);
                    event.jbutton.state = SDL_RELEASED;
                    
                    Nedrysoft::Input::getInstance()->handleEvent(event);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    event.type = SDL_JOYBUTTONUP;
                    event.jbutton.button = static_cast<uint8_t>(Nedrysoft::JoystickButton::Start);
                    event.jbutton.state = SDL_RELEASED;

                    Nedrysoft::Input::getInstance()->handleEvent(event);
                }
 
                break;
            }
        }
    }

    return true;
}

#pragma clang diagnostic pop