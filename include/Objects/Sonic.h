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

#ifndef NEDRYSOFT_SONIC_H
#define NEDRYSOFT_SONIC_H

#include "AnimationPlayer.h"
#include "Animations.h"
#include "Object.h"
#include "Structs.h"
#include "TileMap.h"

#include <SDL2/SDL.h>
#include <functional>

namespace Nedrysoft {
    class Camera;
    class Image;

    enum class GroundMode {
        Floor = 0,
        RightWall = 1,
        Ceiling = 2,
        LeftWall = 3,

        Undefined = 5
    };

    enum class CollisionState {
        Ignore,
        ZeroPushing,
        ZeroWaiting
    };

    /**
     * @brief       The "states" that sonic can be in.  The bottom 4 bits contain information that is used to create
     *              meta-states, namely these bits contain whether sonic is in ground or air state and whether
     *              the state is mirrored or flipped.
     *
     *              By definition, a mirrored state means sonic is left facing and non-mirrored right.
     *
     *              The bits from position 4 upwards define the state such as walking or jumping, when combined with
     *              the lower 4 bits a single state now forms a new meta-state that tells you whether sonic
     *              is in the or ground, what direction he's facing and what what he's currently doing (i.e jumping
     *              or standing still)
     *
     *              By encoding the state this way, it becomes very easy in the code to figure out exactly how
     *              he is behaving.
     */

    constexpr auto PrimaryState(int value) {
        return (1 << value);
    }

    constexpr auto SecondaryState(int primaryMode, int value) {
        return primaryMode | (((value+1) << 4));
    }

    enum class SonicStateFlags {
        Null = 0,

        Ground = PrimaryState(0),
        Air = PrimaryState(1),

        Mirrored = PrimaryState(2),
        /*Flipped = PrimaryState(3),*/

        // meta states

        BallOnGround = SecondaryState(Ground, 1),
        Jumping = SecondaryState(Air, 2),
        Pushing = SecondaryState(Ground, 3),
        StandingOnObject = SecondaryState(Ground, 4),
        Waiting = SecondaryState(Ground, 5),
        StandingOnGround = SecondaryState(Ground, 6),
        Falling = SecondaryState(Air, 7),
        Walking =  SecondaryState(Ground, 8),
        Stopping = SecondaryState(Ground, 9),
        Running =  SecondaryState(Ground, 10),
        Balancing = SecondaryState(Ground, 11),
        BallInTunnel = SecondaryState(Ground, 12),
        BallInAir = SecondaryState(Air, 13),
        Dying = SecondaryState(Air, 14),
        Hurt = SecondaryState(Air, 15),
        Rocket = SecondaryState(Air, 16),
        Dead = SecondaryState(Air, 17),

        // meta-meta states

        BallOnGroundLeft = (BallOnGround | Mirrored),
        BallOnGroundRight = (BallOnGround),

        JumpingLeft = (Jumping | Mirrored),
        JumpingRight = (Jumping),

        PushingLeft = (Pushing | Mirrored),
        PushingRight = (Pushing),

        StandingOnObjectLeft = (StandingOnObject | Mirrored),
        StandingOnObjectRight = (StandingOnObject),

        WaitingLeft = (Waiting | Mirrored),
        WaitingRight = (Waiting),

        StandingOnGroundLeft = (StandingOnGround | Mirrored),
        StandingOnGroundRight = (StandingOnGround),

        FallingLeft = (Falling | Mirrored),
        FallingRight = (Falling),

        WalkingLeft = (Walking | Mirrored),
        WalkingRight = (Walking),

        StoppingLeft = (Stopping | Mirrored),
        StoppingRight = (Stopping),

        RunningLeft = (Running | Mirrored),
        RunningRight = (Running),

        BalancingLeft = (Balancing | Mirrored),
        BalancingRight = (Balancing),

        BallInTunnelLeft = (BallInTunnel | Mirrored),
        BallInTunnelRight = (BallInTunnel),

        BallInAirLeft = (BallInAir | Mirrored),
        BallInAirRight = (BallInAir),

        HurtLeft = (Hurt | Mirrored),
        HurtRight = (Hurt),

        RocketLeft = (Rocket | Mirrored),
        RocketRight = (Rocket)
    };

    inline constexpr SonicStateFlags operator & (SonicStateFlags x, SonicStateFlags y) {
        return static_cast<SonicStateFlags>(static_cast<int>(x) & static_cast<int>(y));
    }

    inline constexpr SonicStateFlags operator | (SonicStateFlags x, SonicStateFlags y) {
        return static_cast<SonicStateFlags>(static_cast<int>(x) | static_cast<int>(y));
    }

    inline constexpr SonicStateFlags & operator |= (SonicStateFlags & x, SonicStateFlags y) {
        x = x | y;

        return x;
    }

    inline constexpr SonicStateFlags operator ^ (SonicStateFlags x, SonicStateFlags y) {
        return static_cast<SonicStateFlags>(static_cast<int>(x) ^ static_cast<int>(y));
    }

    inline constexpr SonicStateFlags operator ~ (SonicStateFlags x) {
        return static_cast<SonicStateFlags>(~static_cast<int>(x));
    }

    constexpr auto sonicState(SonicStateFlags state) {
        return state & ~(SonicStateFlags::Mirrored);
    }

    constexpr auto sonicInState(SonicStateFlags sourceState, SonicStateFlags destinationState) {
        return (static_cast<int>(sourceState) >> 4) == (static_cast<int>(destinationState) >> 4);
    }

    constexpr auto sonicOnGround(SonicStateFlags state) {
        return (state & SonicStateFlags::Ground) == SonicStateFlags::Ground;
    }

    constexpr auto sonicInAir(SonicStateFlags state) {
        return (state & SonicStateFlags::Air) == SonicStateFlags::Air;
    }

    constexpr auto sonicFacingLeft(SonicStateFlags state) {
        return (state & SonicStateFlags::Mirrored) == SonicStateFlags::Mirrored;
    }

    constexpr auto sonicFacingRight(SonicStateFlags state) {
        return (state & SonicStateFlags::Mirrored) != SonicStateFlags::Mirrored;
    }

    enum class AnimationState {
        Normal,
        Mirrored,
        FollowExisting
    };

    enum class SensorPosition {
        None,
        Left,
        Right,
        Centre
    };

    enum {
        FirstTunnel         = 0x1F,
        SecondTunnel        = 0x20,
        LoopForeground      = (0x35 + FlaggedChunk),
        LoopBackground      = (0x36 + FlaggedChunk)
    };

    enum GroundSteepness {
        OutOfRange,
        Shallow,
        HalfSteep,
        FullSteep
    };

    /**
     * @brief            This class manages the main sonic character in the game, it manages the inputs from the controller
     *                   to move the character through the tilemap, colliding with the terrain and objects.
     *
     *                   This class represents the bulk of the game logic as it directly controls where sonic is inside
     *                   the level and the camera class uses the players position to position the camera viewport for
     *                   rendering.
     */
    class Sonic {
        public:
            enum ReboundMode {
                Monitor,
                Badnik,
                Boss
            };

        public:
            /**
             * @brief           Returns the singleton instance of the Sonic object.
             *
             * @returns         the sonic object.
             */
            static auto getInstance() -> Sonic *;

            /**
             * @brief           Sets the initial position in the map of Sonic.
             *
             * @details         Each level defines the initial starting position of Sonic, this function is called
             *                  before the player starts to play the level to set the initial position that is included
             *                  in the level data.
             *
             * @param[in]       position the position in pixels of the starting position of Sonic.
             */
            auto setInitialPosition(const Vector &position) -> void;

            /**
             * @brief           Updates Sonic for the current frame.
             *
             * @details         This function represents the core of the logic for handling Sonic. This function needs to
             *                  be called once every frame, it will update sonics position from control inputs and manages
             *                  his state, tracking collisions and his location in the map.
             *
             * @param[in]       tileMap the terrain tilemap.
             * #param[in]       camera the camera object.
             */
            auto update(TileMap *tileMap, Camera *camera) -> void;

            /**
             * @brief           Returns the current rectangle occupied by Sonic.
             *  
             * @returns         the rectangle.
             */
            auto rect() -> Rect;

            /**
             * @brief           Returns Sonics position in the level.
             *
             * @returns         the vector containing his position.
             */
            auto position() -> Vector;

            /**
             * @brief           Renders the debug overlay.
             *
             * @details         The debug overlay is used to visualise Sonics various data, the actual content
             *                  will vary during development, but generally speaking it will show his rectangle
             *                  and possibly sensors.  This is drawn after all other objects.
             *
             * @param[in]       camera the camera viewport.
             */
            auto renderDebug(Camera *camera) -> void;

            /**
             * @brief           Renders sonic.
             *
             * @param[in]       camera the camera viewport to render against.
             */
            auto render(Camera *camera) -> void;

            /**
             * @brief           Returns the current ground speed of Sonic.
             *
             * @returns         the ground speed.
             */
            [[nodiscard]] auto groundSpeed() const -> float;

            /**
             * @brief           Returns the maximum ground speed of Sonic.
             *
             * @returns         the maximum ground speed.
             */
            [[nodiscard]] auto maxGroundSpeed() const -> float;

            /**
             * @brief           Returns the current vertical speed of Sonic.
             *
             * @returns         the vertical speed.
             */
            [[nodiscard]] auto verticalSpeed() const -> float;

            /**
             * @brief           Returns the current horizontal speed of Sonic.
             *
             * @returns         the horizontal speed.
             */
            [[nodiscard]] auto horizontalSpeed() const -> float;

            /**
             * @brief           Returns whether sonic is currently attached to the ground.
             *
             * @returns         true is sonic is attached to the ground, otherwise false.
             */
            [[nodiscard]] auto onGround() const -> bool;

            /**
             * @brief           Returns the hitbox for Sonic.
             * @returns         the hitbox.
             */
            auto hitBox() -> SDL_Rect;

            /**
             * @brief           Sets the object that Sonic is standing on.
             *
             * @param[in]       object the object to set; or nullptr to clear.
             */
            auto setStandingOnObject(Object *object) -> void;

            /**
             * @brief           Returns the object that Sonic is standing on.
             *
             * @returns         the object or nullptr if he is not standing on an object.
             */
            auto standingOnObject() -> Object *;

            /**
             * @brief           Moves sonic by the given delta values.
             *
             * @param[in]       deltaX the number of pixels to move in the x plane.
             * @param[in]       deltaY the number of pixels to move in the y plane.
             */
            auto move(float deltaX, float deltaY) -> void;

            /**
             * @brief           Returns true of sonic is jumping upwards.
             *
             * @return          true if he is in the jumping state with a negative Y speed; otherwise false.
             */
            auto isJumpingUp() -> bool;

            /**
             * @brief           Returns true of sonic is jumping downwards.
             *
             * @return          true if he is in the jumping state with a positive Y speed; otherwise false.
             */
            auto isJumpingDown() -> bool;

            /**
             * @brief           Moves Sonic into a collision to his right.
             *
             * @details         This function is used by general objects to put sonic in a solid collision on the right
             *                  hand side, if shouldPush is set then sonic will enter pushing or waiting state depending
             *                  on whether the right button is held down.  If shouldPush is false, then he will enter
             *                  falling state.
             *
             * @param[in]       collisionX the point where the collision occured.
             * @param[in]       shouldPush if set then enter pushing or waiting state; otherwise enter falling state.
             */
            auto setObjectCollidedRight(int collisionX, bool shouldPush = true) -> void;

            /**
             * @brief           Moves Sonic into a collision to his left.
             *
             * @details         This function is used by general objects to put sonic in a solid collision on the left
             *                  hand side, if shouldPush is set then sonic will enter pushing or waiting state depending
             *                  on whether the left button is held down.  If shouldPush is false, then he will enter
             *                  falling state.
             *
             * @param[in]       collisionX the point where the collision occured.
             * @param[in]       shouldPush if set then enter pushing or waiting state; otherwise enter falling state.
             */
            auto setObjectCollidedLeft(int collisionX, bool shouldPush = true) -> void;

            /**
             * @brief           Moves Sonic into a collision to his top.
             *
             * @details         this function is used by general objects to put sonic in a solid collision on the top
             *
             * @param[in]       distance the distance which to push Sonic out by.
             */
            auto setObjectCollidedTop(int distance) -> void;

            /**
             * @brief           Returns a string that describes the Sonics current state.
             *
             * @returns         the state string.
             */
            auto currentState() -> std::string;

            /**
             * @brief           Returns offset from the sprite centre point to it's origin.
             * 
             * @param[out]      x the X offset.
             * @param[out]      y the Y offset.
             */
            auto centreOffset(int *x, int *y) -> void;

            /**
             * @brief           Moves sonic to jumping mode.
             *
             * @param[in]       jumpForceY the jump force for the Y direction.
             */
            auto setJumping(float jumpForceY) -> void;

            /**
             * @brief           Starts the control lock for the given duration.
             *
             * @param[in]       time the number of frames to lock the controls for.
             */
            auto setControlLock(int time) -> void;

            /**
             * @brief           Returns how many frames of invulnerability remain.
             *
             * @note            If sonic is not invulnerable then this function returns 0.
             *
             * @returns         A positive number if sonic is currently invulnerable; otherwise 0.
             */
            [[nodiscard]] auto invulnerability() const -> int;

            /**
             * @brief           Moves sonic to the hurt state.
             *
             * @note            The original game has a specific behaviour with spikes in that if sonic lands on them
             *                  while he's already hurt, then he dies instantly.
             *
             * @param[in]       yDistance the vertical distance Sonic needs to be pushed from the object.
             * @param[in]       objectPosition the position of the object that hurt sonic.
             * @param[in]       overrideInvulnerability true if the current invulnerabiity should be ignored.
             */
            auto hurt(float yDistance, Vector objectPositionm, bool overrideInvulnerability = false) -> void;

            /**
             * @brief           Moves sonic to the killed state.
             */
            auto kill() -> void;

            /**
             * @brief           Returns whether sonic is in the dying state.
             *
             * @returns         true if dying; otherwise false.
             */
            auto isDying() -> bool;

            /**
             * @brief           Returns whether sonic is dead.
             *
             * @returns         true if dying; otherwise false.
             */
            auto isDead() -> bool;

            /**
             * @brief           Returns whether sonic is in the hurt state.
             *
             * @returns         true if hurting; otherwise false.
             */
            auto isHurt() -> bool;

            /**
             * @brief           Adds the given number of rings to Sonic.
             *
             * @param[in]       rings the number of rings to add.
             */
             auto addRings(int rings) -> void;

            /**
             * @brief           Returns the number of rings held by Sonic.
             *
             * @returns         the total number of rings.
             */
             [[nodiscard]] auto rings() const -> int;

            /**
             * @brief           Adds the given number of points to Sonic.
             *
             * @param[in]       rings the number of points to add.
             */
             auto addPoints(int points) -> void;

            /**
             * @brief           Returns the number of points accumulated by Sonic.
             *
             * @returns         the total number of points.
             */
             [[nodiscard]] auto points() const -> int;

            /**
             * @brief           Adds the given number of lives to Sonic.
             *
             * @param[in]       lives the number of lives to add.
             */
             auto addLives(int lives) -> void;

            /**
             * @brief           Returns the number of lives Sonic has remaining.
             *
             * @returns         the total number of lives.
             */
             [[nodiscard]] auto lives() const -> int;

            /**
             * @brief           Scatters any collected rings.
             */
            auto scatterRings() -> void;

            /**
             * @brief           Returns whether Sonic is currently in an attacking state.
             *
             * @returns         true if attacking; otherwise false.
             */
            [[nodiscard]] auto attacking() const -> bool;

            /**
             * @brief           Rebound Sonic off an object (if necessary).
             *
             * @details         Sonic may not always rebound depending on how he collided with the object, the
             *                  object will call this function when it knows sonic has collided with it and Sonic
             *                  will then decide if a rebound needs to happen.
             *
             * @param[in]       mode the type of rebound to be used.
             * @param[in]       objectPosition the position of the object that has hit sonic.
             */
            auto rebound(ReboundMode mode, Vector objectPosition) -> void;

            auto rocket(float xDistance, float yDistance, float xSpeed, float ySpeed, int controlLockFrames) -> void;

        private:
            /**
             * @brief           Constructs a new Sonic object.
             */
            Sonic();

            /**
             * @brief           Destroys the Sonic object.
             */
            ~Sonic();

            /**
             * @brief           Performs the per frame update when Sonic is on the ground.
             *
             * @details         When sonic is attached to the ground, this function is called each frame to update Sonic
             *                  with regards to control inputs and handle collisions with the tilemap and any object
             *                  collisions that are triggered by Sonic.
             *
             *                  In ground mode sonic will track the ground, moving up and down with the contour, ground
             *                  mode is unique in that it deals with positive collision distances in addition to the more
             *                  normal negative distances which signify a collision.
             *
             * @param[in]       tileMap The foreground tilemap.
             */
            auto updateNormalState(TileMap *tileMap, Camera *camera) -> void;

            /**
             * @brief           Performs the per frame update when Sonic is in the dying state.
             *
             * @details         The player has no control when Sonic is in the dying state, for convenience of handling
             *                  the intricacies we just handle dying in it's own update routine.
             *
             * @param[in]       tileMap The foreground tilemap.
             * @param[in]       camera The camera object.
             */
            auto updateDyingState(TileMap *tileMap, Camera *camera) -> void;

            /**
             * @brief           Performs the per frame update when Sonic is in the hurt state.
             *
             * @param[in]       tileMap The foreground tilemap.
             * @param[in]       camera The camera object.
             */
            auto updateHurtState(TileMap *tileMap, Camera *camera) -> void;

            /**
             * @brief           Performs the per frame update when Sonic is rolling.
             *
             * @details         This is a special case that is used to track sonic in a rolling state, this is either
             *                  in "Normal" mode or "Airborne".  This is handled outside of the "Normal" and "Airborne"
             *                  modes to make the code easier to maintain as the rolling state differs from normal
             *                  handling in the aforementioned states.
             *
             * @param[in]       tileMap The foreground tilemap.
             * @param[in]       camera The camera object.
             */
            auto updateRollingState(TileMap *tileMap, Camera *camera) -> void;

            /**
             * @brief           Performs the per frame update when Sonic is airborne.
             *
             * @details         When sonic is not attached to the ground, this state tracks his movement through the air
             *                  until he collides with the ground.  Once Sonic has collided with the ground, his ground
             *                  velocity is calculated from his current airborne state and he is then returned to
             *                  normal ground mode.
             *
             * @param[in]       tileMap The foreground tilemap.
             * @param[in]       camera The camera object.
             */
            auto updateAirborneState(TileMap *tileMap, Camera *camera) -> void;

            /**
             * @brief           Checks Sonics wall sensors for terrain.
             *
             * @details         This function is used to find terrain either to the left or the right of Sonic.  If
             *                  a solid tile is found, then this function returns true and fills in the parameters
             *                  that allow Sonic to react to the tile.
             *
             *                  In most situations if a solid tile is found we generally are only interested in the result
             *                  when the distance is negative, a negative distance indicates that we have collided with
             *                  the terrain and are inside it, so Sonic needs to be pushed back by the distance so that
             *                  he does not disappear into it.  There are some situations where we are interested in
             *                  positive results, but these are not common cases.
             *
             *                  In addition to distance, we also get told which of the sensors sound the solid tile,
             *                  Sonic will only test the wall sensor in the direction he is moving, so if moving left
             *                  he will not test the right hand wall sensor.
             *
             * @param[in]       tileMap The tilemap to check collisions against.
             * @param[in]       groundMode The current ground mode of sonic.
             * @param[out]      activatedSensor The sensor that triggered on a solid tile.
             * @param[out]      distance The distance to the solid tile.
             * @param[in]       sensorOffset An adjustment that may be required according to when this function is called relative
             *                  to moving sonic.
             *
             * @returns         True if a solid tile was found; otherwise false.
             */
            auto checkWallSensors(TileMap *tileMap, GroundMode groundMode, SensorPosition *activatedSensor, float *distance, const Vector &sensorOffset = Vector(0, 0)) -> bool;

            /**
             * @brief           Checks Sonics ground sensors for terrain.
             *
             * @details         This function is used to find terrain either to the left or the right sides of Sonic
             *                  pointing towards the ground.  As sonic has multiple ground modes (required for the loops
             *                  ground is not guaranteed to be straight down, in left wall, right wall and ceiling modes
             *                  the ground is not where you think it is!
             *
             *                  If a solid tile is found, then this function returns true and fills in the parameters
             *                  that allow Sonic to react to the tile.
             *
             *                  The ceiling sensors are only really active when sonic is about to jump, the ceiling
             *                  sensors can prevent him from jumping in too low a place, additionally they allow him
             *                  to ground with ceiling "ground" if he has a high enough velocity.
             *
             *                  In addition to distance, we also get told which of the ground sensors sound the solid tile.
             *
             *                  The distance is always the smallest one that is returned.
             *
             * @param[in]       tileMap The tilemap to check collisions against.
             * @param[out]      activatedSensors the first entry is the sensor that triggered the ground, the remaining entries tell us what other
             *                  sensors were activated.
             * @param[out]      distance The distance to the solid tile.
             * @param[out]      groundAngle The angle of the ground if solid.
             * @param[in]       sensorOffset An adjustment that may be required according to when this function is called relative
             *                  to moving sonic.
             *
             * @returns         True if a solid tile was found; otherwise false.
             */
            auto checkGroundSensors(TileMap *tileMap, SensorPosition *activatedSensors, float *distance, float *groundAngle, const Vector &sensorOffset = Vector(0, 0)) -> bool;

            /**
             * @brief           Checks Sonics ceiling sensors for terrain.
             *
             * @details         This function is used to find terrain either to the left or the right of Sonic.  If
             *                  a solid tile is found, then this function returns true and fills in the parameters
             *                  that allow Sonic to react to the tile.
             *
             *                  The ground sensors are nearly always active and searching for ground.  There is an exception
             *                  to this when solid is standing on an object in the level, when this state is true the
             *                  ground sensors are disabled as they would interfere with the object collision, so while sonic
             *                  is standing on an object, the object is responsible for sonics ground collision and
             *                  exiting that state.
             *
             *                  In addition to distance, we also get told which of the ground sensors sound the solid tile.
             *
             *                  The distance is always the smallest one that is returned.
             *
             * @param[in]       tileMap The tilemap to check collisions against.
             * @param[out]      activatedSensor The sensor that triggered on a solid tile.
             * @param[out]      distance The distance to the solid tile.
             * @param[out]      groundAngle The angle of the ground if solid.
             * @param[in]       sensorOffset An adjustment that may be required according to when this function is called relative
             *                  to moving sonic.
             *
             * @returns         True if a solid tile was found; otherwise false.
             */
            auto checkCeilingSensors(TileMap *tileMap, GroundMode groundMode) -> void;

            /**
             * @brief           Returns the current ground mode.
             *
             * @details         Sonic has the ability to run up walls and perform loops, this requires that he has
             *                  the concept of momentum and that when travelling fast enough, we can stick to the walls
             *                  or move round loops without falling off.
             *
             *                  In order to do this, the engine has to dynamically re-orient where ground is.
             *
             *                  Normally ground is pointing straight down, this is Floor Mode.  There is a ceiling mode
             *                  where ground is pointing up, this is the mode sonic enters when traversing the top of
             *                  a loop.  Finally there is left and right wall modes, these mode allow sonic to traverse
             *                  concave and convex surfaces and remain stuck to them.
             *
             * @returns         The ground mode.
             */
            [[nodiscard]] auto groundMode() const -> GroundMode;

            /**
             * @brief           An alternative movement mode that is entered by the right trigger.
             *
             * @details         When active, sonic can be moved around the map using the joy pad and then dropped back
             *                  into the level.
             *
             * @param[in]       tileMap The tilemap.
             */
            auto updatePositionDebug(TileMap *tileMap) -> void;

            /**
             * @brief           Replaces the current state with a new settings.
             *
             * @details         Sonics state determines how he is drawn and how he interacts with the environment, the
             *                  state includes his direction, whether he's grounded and what he's currently doing.
             *
             *                  This function allows us to move from one state to another, unlike modifyState that
             *                  maintains his grounded state and direction, calling this function will replace those with
             *                  new values, so it's important that those are passed into this function when it is called
             *                  otherwise he will start behaving erratically.
             */
             auto setState(SonicStateFlags state) -> void;

            /**
             * @brief           Modifies the current state of Sonic.
             *
             * @details         Sonics current state is built from a number of different parts, these tell us if he's on the
             *                  ground, in the air or facing left or right.  When combined with other values, this allows
             *                  us to take a state value and discover if he's in the air, on the ground and which way he
             *                  is facing alongside what he's doing, running, waiting, jumping etc.
             *
             *                  Often we change what he's doing, but we need to keep his direction or grounded state, so
             *                  this modify function allows us to switch what he's doing without affecting his direction
             *                  or ground state.  This allows us to easily transition from states such as walking to running.
             */
            auto modifyState(SonicStateFlags state) -> void;

            /**
             * @brief           Detaches the player from the ground (if they're currently on the ground).
             *
             * @details         This is used when the player has a ground speed of zero but is on steep ground, by
             *                  detaching the player from the ground, they move to air state and will reattach to the
             *                  ground but with a steep angle, causing sonic to fall down the slope.
             *
             *                  It's also used in ceiling mode to make sonic fall if he's going too slow.
             */
            auto detachFromGround() -> void;

            /**
             * @brief           Returns whether the player is primarily moving horizontally.
             *
             * @details         Check's whether the absolute X speed is greater than the absolute Y speed.
             *
             * @returns         True if moving mostly horizontally, otherwise false.
             */
            [[nodiscard]] auto movingMostlyHorizontal() const -> bool;

            /**
             * @brief           Returns the steepness of the ground.
             *
             * @returns         The ground steepness.
             */
            [[nodiscard]] auto groundSteepness() const -> GroundSteepness;

            /**
             * @brief           Returns a string that describes the Sonics current state.
             *
             * @details         Sonics state includes flags for his direction and also whether he is in the air
             *                  or grounded.  THe SonicStateFlags enum contains these values, this function decodes the
             *                  state flags passed in and returns a human readable string that describes the state.
             *
             * @param[in]       state The state flags to decode.
             *
             * @returns         A human readable string of the state.
             */
            static auto stateString(SonicStateFlags state) -> std::string;

            /**
             * @brief           Returns the current ground mode string.
             *
             * @details         Sonics is generally in floor mode, but when he interacts with loops and half pipes then
             *                  the orientation of the ground may change, this function can be used to retrieve a human
             *                  readable string describing the current ground mode. (Floor, Right Wall, Left Wall, Ceiling)
             *
             * @returns         A string describing the current ground mode.
             */
            auto groundModeString() -> std::string;

            /**
             * @brief           Clips the current position of Sonic to the tilemap limits.
             *
             * @details         Ensures that Sonic remains in the bounds of the tile map, it ensures that he cannot
             *                  walk off any side of the map.  As the size of his sprite can change according to the
             *                  current animation frame being displayed, this function clips to the current animation
             *                  frame that ensures that he is always fully on screen.
             *
             * @param[in]       tileMap The tilemap to clip against.
             */
            auto clipPosition(TileMap *tileMap) -> void;

            /**
             * @brief           Returns the current width of Sonic.
             *
             * @details         Sonics width varies according to the animation frame currently being displayed, this
             *                  function will return the width of the current frame.
             *
             * @returns         The width of the current frame in pixels.
             */
            auto width() -> float;

            /**
             * @brief           Returns the current height of Sonic.
             *
             * @details         Sonics height varies according to the animation frame currently being displayed, this
             *                  function will return the height of the current frame.
             *
             * @returns         The height of the current frame in pixels.
             */
            auto height() -> float;

            /**
             * @brief           Reattaches Sonic back to ground using the given delta vector.
             *
             * @param[in]       dx The additional X delta to move sonic when reattaching to ground.
             * @param[in]       dy The additional Y delta to move sonic when reattaching to ground.
             */
            auto reattachToGround(int dx, int dy) -> void;

            /**
             * @brief       Asserts if the state is not valid.
             */
            auto checkStateValid() -> void;

            /**
             * @brief           Draws a sensor.
             * 
             * @details         Used to draw a sensor on screen, the sensor has an outline to make it more visible.
             *
             * @param[in]       x       The x coordinate of the sensor.
             * @param[in]       y       The y coordinate of the sensor.
             * @param[in]       red     The red component.
             * @param[in]       green   The green component.
             * @param[in]       blue    The blue component.
             * @param[in]       alpha   The alpha component.
             * @param[in]       camera  The camera component.
             */
            auto drawSensor(float x, float y, int8_t red, uint8_t green, uint8_t blue, uint8_t alpha, Camera *camera) -> void;

        private:
            float m_x;                                  //!< Current X position.
            float m_y;                                  //!< Current Y position.

            float m_groundSpeed;                        //!< Current ground speed.
            float m_groundAngle;                        //!< Current ground angle.

            float m_xSpeed;                             //!< Current X speed.
            float m_ySpeed;                             //!< Current Y speed.

            float m_maxGroundSpeed;                     //!< Maximum X speed.

            int m_controlLockTimer;                     //!< Control lock timer (non-zero if active)

            Object *m_standingOnObject;

            AnimationPlayer m_currentAnimation;

            SonicStateFlags m_currentState;

            Animations *m_animations;

            Vector m_initialPosition;

            Image *m_wallSensorTexture;

            int m_lastChunkId;

            int m_invulnerabilityTimer;

            int m_hurtTimer;

            int m_rings;
            int m_points;
            int m_lives;

            int m_dyingTimer;
    };
}

#endif //NEDRYSOFT_SONIC_H
