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


#include "Sonic.h"

#include "Animation.h"
#include "AnimationStep.h"
#include "Audio.h"
#include "Camera.h"
#include "Color.h"
#include "DebugManager.h"
#include "GameRenderer.h"
#include "Hud.h"
#include "Image.h"
#include "Input.h"
#include "ObjectsManager.h"
#include "ScatteredRing.h"
#include "Utils.h"

#include <SDL_mixer.h>

/**
 * ground coefficients
 */

constexpr auto SonicGroundAcceleration = 0.046875f;
constexpr auto SonicGroundDeceleration = 0.5f;
constexpr auto SonicGroundFriction = SonicGroundAcceleration;
constexpr auto SonicGroundSlopeFactor = 0.125f;
constexpr auto SonicGroundSlopeRollingUpHill = 0.078125f;
constexpr auto SonicGroundSlopeRollingDownHill = 0.3125f;
constexpr auto SonicGroundRollFrictionSpeed = 0.0234375f;
constexpr auto SonicGroundRollDecelerationSpeed = 0.125f;

/**
 * airborne coefficients
 */

constexpr auto SonicAirborneAcceleration = SonicGroundAcceleration * 2.0f;
constexpr auto SonicAirborneJumpForceCoefficient = 6.5f;
constexpr auto SonicAirborneGravityCoefficient = 0.21875f;
constexpr auto SonicAirborneDragCoefficient = 0.125f;
constexpr auto SonicAirborneHurtGravityCoefficient = 0.1875f;

/**
 * constants
 */
constexpr auto SonicGroundSpeedMax = 6.0f;
constexpr auto SonicPushRadius = 10.0f;
constexpr auto SonicGroundZeroAngleAdjustment = 2.8125f;

constexpr auto SonicFallingControlLockTimer = 30;
constexpr auto SonicSpringControlLockTimer = 15;

constexpr auto SonicMaximumXSpeed = 16.0f;
constexpr auto SonicMaximumYSpeed = 16.0f;

constexpr auto SonicSkidVelocity = 4.0f;

constexpr auto ScatteredRingAngle = 101.25f;
constexpr auto ScatteredRingAngleSpacing = 22.5f;
constexpr auto ScatteredRingsMaximum = 32;
constexpr auto ScatteredRingsInnerVelocity = 2.0f;
constexpr auto ScatteredRingsOuterVelocity = 4.0f;

constexpr auto HurtInvulnerabilityFrames = 120;

constexpr auto SonicJumpHeightAdjustment = 4.0f;

/**
 * any useful functions!
 */

constexpr float snapToGround(float newAngle, float currentAngle) {
    return (newAngle == -1) ? fmodf(((floorf((currentAngle / 90.0f) + 0.5f)) * 90.0f), 360.0f) : newAngle;
}

Nedrysoft::Sonic::Sonic() :
        m_groundSpeed(0.0f),
        m_groundAngle(0.0f),
        m_x(0.0f),
        m_y(0.0f),
        m_xSpeed(0.0f),
        m_ySpeed(0.0f),
        m_currentState(SonicStateFlags::Null),
        m_currentAnimation(),
        m_animations(),
        m_maxGroundSpeed(SonicGroundSpeedMax),
        m_initialPosition(0.0f, 0.0f),
        m_controlLockTimer(0),
        m_standingOnObject(nullptr),
        m_invulnerabilityTimer(0),
        m_hurtTimer(0),
        m_lastChunkId(0),
        m_rings(0),
        m_points(0),
        m_lives(3) {

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    m_animations = Animations::load(gameRenderer->renderer(), "./data/art/objects/sonic/sonic.json");

    setState(Nedrysoft::SonicStateFlags::Waiting);

    m_wallSensorTexture = new Image(gameRenderer->renderer(), "./data/wall-sensor.png", "PNG");
}

Nedrysoft::Sonic::~Sonic() = default;

auto Nedrysoft::Sonic::setInitialPosition(const Vector &position) -> void {
    m_initialPosition = position;

    m_x = m_initialPosition.x();
    m_y = m_initialPosition.y();

    m_groundSpeed = 0;
    m_groundAngle = 0;
    m_xSpeed = 0;
    m_ySpeed = 0;
    m_currentState = SonicStateFlags::Null;
    m_hurtTimer = 0;
    m_lastChunkId = 0;
    m_rings = 0;
    m_points = 0;
    m_invulnerabilityTimer = 0;
    m_standingOnObject = nullptr;
    m_controlLockTimer = 0;
}

auto Nedrysoft::Sonic::rect() -> Rect {
    return {
        static_cast<int>(round(m_x) - (width() / 2.0f)),
        static_cast<int>(round(m_y) - (height() / 2.0f)),
        static_cast<int>(width()),
        static_cast<int>(height())
    };
}

auto Nedrysoft::Sonic::getInstance() -> Sonic * {
    static Sonic instance;

    return &instance;
}

auto Nedrysoft::Sonic::update(Nedrysoft::TileMap *tileMap, Camera *camera) -> void {
    auto input = Input::getInstance();

    /**
     * The jump button must be released
     */

    if (!Nedrysoft::Input::getInstance()->enabled(JoystickButton::A)) {
        if (!Nedrysoft::Input::getInstance()->pressed(JoystickButton::A)) {
            if (!sonicInState(m_currentState, SonicStateFlags::Jumping)) {
                Nedrysoft::Input::getInstance()->setEnabled(JoystickButton::A, true);
            }
        }
    }

    if (input->pressed(JoystickButton::RightTrigger)) {
        //updatePositionDebug(tileMap);

        return;
    }

    auto oldState = m_currentState;

    switch(tileMap->chunkIndex(m_x, m_y)) {
        case FirstTunnel:
        case SecondTunnel: {
            if (sonicOnGround(m_currentState)) {
                modifyState(SonicStateFlags::BallInTunnel);

                updateRollingState(tileMap, camera);

                Nedrysoft::Audio::getInstance()->playSample(0, Nedrysoft::SoundId::Roll);
            }

            break;
        }

        case LoopForeground: {
            if (groundMode() == Nedrysoft::GroundMode::Ceiling) {
                if (groundSpeed() > 0.0f) {
                    tileMap->setActivePlane(TileMapPlane::Background);
                }
            } else {
                float x = static_cast<float>((static_cast<int>(m_x / 256.0f) * 256)) + 128.0f;

                /**
                 * check if we have just entered the loop tile, if we have then we check which side of it
                 * we entered from and set the foreground or background map appropriately.
                 */

                if ((m_lastChunkId != LoopForeground) && (m_lastChunkId != LoopBackground)) {
                    if (m_x < x) {
                        tileMap->setActivePlane(TileMapPlane::Foreground);
                    } else {
                        tileMap->setActivePlane(TileMapPlane::Background);
                    }
                }
            }

            break;
        }

        case LoopBackground: {
            if (groundMode() == Nedrysoft::GroundMode::Ceiling) {
                if (groundSpeed() < 0.0f) {
                    tileMap->setActivePlane(TileMapPlane::Foreground);
                }
            } else {
                float x = static_cast<float>((static_cast<int>(m_x / 256.0f) * 256)) + 128.0f;

                /**
                 * check if we have just entered the loop tile, if we have then we check which side of it
                 * we entered from and set the foreground or background map appropriately.
                 */

                if ((m_lastChunkId != LoopForeground) && (m_lastChunkId != LoopBackground)) {
                    if (m_x < x) {
                        tileMap->setActivePlane(TileMapPlane::Foreground);
                    } else {
                        tileMap->setActivePlane(TileMapPlane::Background);
                    }
                }
            }

            break;
        }

        default: {
            if (sonicInState(m_currentState, SonicStateFlags::BallInTunnel)) {
                modifyState(SonicStateFlags::BallOnGround);
            }

            break;
        }
    }

    m_lastChunkId = tileMap->chunkIndex(m_x, m_y);

    if (sonicInState(m_currentState, SonicStateFlags::Dying)) {
        updateDyingState(tileMap, camera);

        return;
    }

    if (sonicInState(m_currentState, SonicStateFlags::Dead)) {
        return;
    }

    if (sonicOnGround(m_currentState)) {
        if ((sonicInState(m_currentState, SonicStateFlags::BallOnGround)) ||
            (sonicInState(m_currentState, SonicStateFlags::BallInTunnel)) ) {
            updateRollingState(tileMap, camera);
        } else {
            updateNormalState(tileMap, camera);
        }
    } else {
        updateAirborneState(tileMap, camera);
    }

    m_currentAnimation.selectAlternative(0);

    if (m_invulnerabilityTimer) {
        m_invulnerabilityTimer--;
    }

/*
    if ((m_groundAngle >= 0) && (m_groundAngle < 45)) {
        m_currentAnimation.selectAlternative(0);
    } else if ((m_groundAngle >= 45) && (m_groundAngle < 90)) {
        m_currentAnimation.selectAlternative(1);
    } else if ((m_groundAngle >= 90) && (m_groundAngle < 135)) {
        m_currentAnimation.selectAlternative(2);
    } else if ((m_groundAngle >= 135) && (m_groundAngle < 225)) {
        m_currentAnimation.selectAlternative(3);
    } else if ((m_groundAngle >= 225) && (m_groundAngle < 270)) {
        m_currentAnimation.selectAlternative(2);
    } else if ((m_groundAngle >= 270) && (m_groundAngle < 315)) {
        m_currentAnimation.selectAlternative(1);
    } else if ((m_groundAngle >= 315) && (m_groundAngle < 360)) {
        m_currentAnimation.selectAlternative(0);
    }
*/
    if (m_currentState == oldState) {
        m_currentAnimation.next();
    } else {
        std::cout <<  color::rize("[mode changed]", "Yellow", "Default", "Bold") +
                    debugValue("new", stateString(m_currentState), "Cyan", "Green", "Bold") +
                    debugValue("previous", stateString(oldState), "Cyan", "Green", "Bold") <<
                    std::endl;
    }
}

auto Nedrysoft::Sonic::updateNormalState(TileMap *tileMap, Camera *camera) -> void {
    N_UNUSED(camera);

    bool skipWallSensors = false;
    auto input = Nedrysoft::Input::getInstance();

    auto jumpButton = false;
    auto leftButton = false;
    auto rightButton = false;
    auto downButton = false;

    if (!m_controlLockTimer) {
        jumpButton = input->pressed(JoystickButton::A) && Nedrysoft::Input::getInstance()->enabled(JoystickButton::A);
        leftButton = input->pressed(JoystickButton::Left);
        rightButton = input->pressed(JoystickButton::Right);
        downButton = input->pressed(JoystickButton::Down);
    }

    auto initialState = m_currentState;

    // 2. if spindash {..}

    // 3. update ground speed with slope factor

    if (m_groundSpeed != 0.0f) {
        m_groundSpeed -= SonicGroundSlopeFactor * sinf(degToRad(m_groundAngle));
    }

    m_groundSpeed = std::max(-SonicGroundSpeedMax, std::min(m_groundSpeed, SonicGroundSpeedMax));

    // 4. check for starting a jump

    if (jumpButton) {
        //if (!downButton) {
            /**
             * now we transition to airborne state.
             */

            m_xSpeed -= SonicAirborneJumpForceCoefficient * sinf(degToRad(m_groundAngle));
            m_ySpeed -= SonicAirborneJumpForceCoefficient * cosf(degToRad(m_groundAngle));

            m_y += SonicJumpHeightAdjustment;

            modifyState(Nedrysoft::SonicStateFlags::Jumping);

            Nedrysoft::Audio::getInstance()->playSample(0, Nedrysoft::SoundId::Jump);

            m_standingOnObject = nullptr;

            //Nedrysoft::Input::getInstance()->setEnabled(JoystickButton::A, false);

            return;
        //}
    } else if (downButton) {
        if (fabs(m_groundSpeed) > 0.5f) {
            modifyState(SonicStateFlags::BallOnGround);
        }

        return;
    }

    // 5. update ground speed with controls

    if (!sonicInState(m_currentState, SonicStateFlags::Pushing)) {
        if ((!leftButton) && (!rightButton)) {
            m_groundSpeed -= std::min<float>(fabs(m_groundSpeed), SonicGroundFriction) * sign(m_groundSpeed);

            if (m_groundSpeed == 0.0f) {
                 modifyState(Nedrysoft::SonicStateFlags::Waiting);
            }
        } else if ((!m_controlLockTimer) && (leftButton)) {
            if (m_groundSpeed > 0.0f) {
                m_groundSpeed -= SonicGroundDeceleration;

                if (m_groundSpeed <= 0) {
                    m_groundSpeed = -0.5f;
                }

                if (m_groundSpeed >= SonicSkidVelocity) {
                    Nedrysoft::Audio::getInstance()->playSample(0, Nedrysoft::SoundId::Skid);

                    setState(Nedrysoft::SonicStateFlags::StoppingRight);
                }
            } else if (m_groundSpeed > -SonicGroundSpeedMax) {
                m_groundSpeed -= SonicGroundAcceleration;
                
                if (m_groundSpeed <= -SonicGroundSpeedMax) {
                    m_groundSpeed = -SonicGroundSpeedMax;
                }

                if (std::round(m_groundSpeed) <= -SonicGroundSpeedMax) {
                    setState(Nedrysoft::SonicStateFlags::RunningLeft);
                } else {
                    setState(Nedrysoft::SonicStateFlags::WalkingLeft);
                }
            }
        } else if ((!m_controlLockTimer) && (rightButton))  {
            if (m_groundSpeed < 0.0f) {

                m_groundSpeed += SonicGroundDeceleration;

                if (m_groundSpeed >= 0) {
                    m_groundSpeed = 0.5;
                }

                if (m_groundSpeed <= -SonicSkidVelocity) {
                    Nedrysoft::Audio::getInstance()->playSample(0, Nedrysoft::SoundId::Skid);

                    setState(Nedrysoft::SonicStateFlags::StoppingLeft);
                }
            } else if (m_groundSpeed < SonicGroundSpeedMax) {
                m_groundSpeed += SonicGroundAcceleration;

                if (m_groundSpeed >= SonicGroundSpeedMax) {
                    m_groundSpeed = SonicGroundSpeedMax;
                }

                if (std::round(m_groundSpeed) >= SonicGroundSpeedMax) {
                    setState(Nedrysoft::SonicStateFlags::RunningRight);
                } else {
                    setState(Nedrysoft::SonicStateFlags::WalkingRight);
                }
            }
        }               
    } else {
        if (sonicFacingLeft(m_currentState)) {
            if (!leftButton) {
                modifyState(SonicStateFlags::Waiting);

                skipWallSensors = true;
            }
        } else if (sonicFacingRight(m_currentState)) {
            if (!rightButton) {
                modifyState(SonicStateFlags::Waiting);

                skipWallSensors = true;
            }
        }

        m_groundSpeed = 0.0f;
    }

    // 6. check for ducking/balancing

    // 7. check for wall collisions

    auto sensorAdjustment = Vector(m_xSpeed, (m_groundAngle == 0.0f ? 8.0f : 0.0f));
    auto activatedSensor = SensorPosition::None;
    auto sensorDistance = 0.0f;

    if ((!sonicInState(m_currentState, SonicStateFlags::Waiting)) &&
        (checkWallSensors(tileMap, groundMode(), &activatedSensor, &sensorDistance, sensorAdjustment))) {

        if (sensorDistance <= 0.0f) {
            CollisionState collisionState = CollisionState::Ignore;

            if ((activatedSensor == SensorPosition::Left) && (sonicFacingLeft(m_currentState))) {
                collisionState = leftButton ? CollisionState::ZeroPushing : CollisionState::ZeroWaiting;
            } else if ((activatedSensor == SensorPosition::Right) && (sonicFacingRight(m_currentState))) {
                collisionState = rightButton ? CollisionState::ZeroPushing : CollisionState::ZeroWaiting;
            }

            if (collisionState != CollisionState::Ignore) {
                move(sensorDistance + m_xSpeed, 0.0f);

                m_groundSpeed = 0.0f;

                modifyState(collisionState == CollisionState::ZeroWaiting ? SonicStateFlags::Waiting : SonicStateFlags::Pushing);
            }
        }
    }

    // 8. check for starting a roll

    // 9. check for screen boundaries and kill plane collisions

    // 10. update x and y speeds

    m_xSpeed = std::max(-SonicMaximumXSpeed, std::min(m_groundSpeed * cosf(degToRad(m_groundAngle)), SonicMaximumXSpeed));
    m_ySpeed = std::max(-SonicMaximumYSpeed, std::min(m_groundSpeed * -sinf(degToRad(m_groundAngle)), SonicMaximumYSpeed));

    m_x += m_xSpeed;
    m_y += m_ySpeed;

    clipPosition(tileMap);

    Vector sensorOffset = {0.0f, 0.0f};
    float distance;

    float angle;

    // 11. check ground sensors

    if (!m_standingOnObject) {
        SensorPosition activatedGroundSensors[] = {SensorPosition::None, SensorPosition::None, SensorPosition::None};

        if (checkGroundSensors(tileMap, activatedGroundSensors, &distance, &angle, sensorOffset)) {
            if ((m_groundSpeed == 0) && (activatedGroundSensors[1] == SensorPosition::None)) {
                if (activatedGroundSensors[0] == SensorPosition::Left) { 
                    setState(SonicStateFlags::BalancingRight);
                } else {
                    setState(SonicStateFlags::BalancingLeft);
                }
            }

            if (distance != 0) {
                m_groundAngle = snapToGround(angle, m_groundAngle);

                //if (fabs(distance) <= 14) {
                    move(0.0f, distance);
                //}
            } else {
                //reattachToGround(0,0);
                move(0.0f, distance);
            }
        } else {
             modifyState(Nedrysoft::SonicStateFlags::Falling);
        }
    }

    static auto lastMode = groundModeString();
    static auto lastAngle = m_groundAngle;

    if ((lastMode != groundModeString() || (lastAngle != m_groundAngle))) {
        lastAngle = m_groundAngle;
        lastMode = groundModeString();
    }

    // 12. check for falling if ground speed is too slow

    if (sonicOnGround(m_currentState)) {
        if (!m_controlLockTimer) {
            if ((abs(m_groundSpeed) < 2.5f) && (m_groundAngle >= 46.0f) && (m_groundAngle <= 315.0f)) {
                m_groundSpeed = 0;

                detachFromGround();

                m_controlLockTimer = SonicFallingControlLockTimer;
            }
        } else {
            m_controlLockTimer--;
        }
    }
}

auto Nedrysoft::Sonic::updateAirborneState(TileMap *tileMap, Camera *camera) -> void {
    N_UNUSED(camera);

    auto input = Nedrysoft::Input::getInstance();

    auto jumpButton = false;
    auto leftButton = false;
    auto rightButton = false;

    if (!m_controlLockTimer) {
        jumpButton = input->pressed(JoystickButton::A);
        leftButton = input->pressed(JoystickButton::Left);
        rightButton = input->pressed(JoystickButton::Right);
    }

    if ((m_currentState & SonicStateFlags::Rocket) == SonicStateFlags::Rocket) {
        if (m_ySpeed >= 0) {
            modifyState(SonicStateFlags::Falling);
        } else {
            if (leftButton) {
                setState(SonicStateFlags::RocketLeft);
            } else if (rightButton) {
                setState(SonicStateFlags::RocketRight);
            }
        }
    }

    // 1. check for variable jump

    if ((!jumpButton) && (m_currentState & SonicStateFlags::Jumping) == SonicStateFlags::Jumping) {
        /*
         * limit y speed to -4, this allows sonics jump to be cut short by releasing the jump button quickly
         */
        m_ySpeed = std::max(-4.0f, m_ySpeed);
    }

    // 2. check for super

    // 3. update x speed on directional input

    if (rightButton) {
        m_xSpeed += SonicAirborneAcceleration;
    } else if (leftButton) {
        m_xSpeed -= SonicAirborneAcceleration;
    }

    m_xSpeed = std::max(-SonicGroundSpeedMax, std::min(m_xSpeed, SonicGroundSpeedMax));

    // 4. apply air drag

    if ((m_ySpeed < 0.0f) && (m_ySpeed > -4.0f)) {
        m_xSpeed -= (m_xSpeed / SonicAirborneDragCoefficient) / 256.0f;
    }

    // NOTE 5. move sonic

    m_x += m_xSpeed;
    m_y += m_ySpeed;

    clipPosition(tileMap);

    // 6. apply gravity

    m_ySpeed += SonicAirborneGravityCoefficient;

    // 7. Check underwater

    // 8. rotate angle back to zero

    if (m_groundAngle != 0.0f) {
        if (m_groundAngle >= 180.0f) {
            m_groundAngle += SonicGroundZeroAngleAdjustment;

            if (m_groundAngle >= 360.0f) {
                m_groundAngle = 0.0f;
            }
        } else {
            m_groundAngle -= SonicGroundZeroAngleAdjustment;

            if (m_groundAngle < 0.0f) {
                m_groundAngle = 0.0f;
            }
        }
    }

    // 9. Check collisions in order, wall, ground and then ceiling

    auto sensorAdjustment = Vector(m_xSpeed - 6, 8.0f);//(m_groundAngle == 0.0f ? 8.0f : 0.0f));
    auto activatedSensor = SensorPosition::None;
    auto sensorDistance = 0.0f;

    if (checkWallSensors(tileMap, groundMode(), &activatedSensor, &sensorDistance, sensorAdjustment)) {
        if (sensorDistance <= 0.0f) {

            if (activatedSensor == SensorPosition::Left) {
                move(-sensorDistance, 0.0f);
            } else if (activatedSensor == SensorPosition::Right) {
                move(sensorDistance, 0.0f);
            }

            m_groundSpeed = m_ySpeed;
            m_xSpeed = 0.0f;
        }
    }

    auto groundAngle = 0.0f;

    if (!m_standingOnObject) {
        SensorPosition activatedGroundSensors[] = {SensorPosition::None, SensorPosition::None, SensorPosition::None};

        if (checkGroundSensors(tileMap, activatedGroundSensors, &sensorDistance, &groundAngle, sensorAdjustment)) {
            auto adjustment = 0.0f;

            if (sonicInState(m_currentState, SonicStateFlags::Jumping)) {
                adjustment = SonicJumpHeightAdjustment;
            }

            if (sensorDistance < 0.0f) {
                if (movingMostlyHorizontal()) {
                    if (m_ySpeed >= 0.0f) {
                        reattachToGround(0, static_cast<int>(sensorDistance - adjustment));
                    }
                } else {
                    if (m_ySpeed > 0.0f) {
                        if (sensorDistance >= -(m_ySpeed + 8.0f)) {
                            reattachToGround(0, static_cast<int>(sensorDistance - adjustment));
                        }
                    }
                }
            }
        }
    }

    if (m_controlLockTimer) {
        m_controlLockTimer--;
    }

    //checkCeilingSensors(tileMap, groundMode());
}

auto Nedrysoft::Sonic::updateRollingState(TileMap *tileMap, Camera *camera) -> void {
    N_UNUSED(camera);

    auto initialGroundSpeed = m_groundSpeed;

    auto input = Nedrysoft::Input::getInstance();

    auto jumpButton = false;
    auto leftButton = false;
    auto rightButton = false;
    auto downButton = false;

    if (!m_controlLockTimer) {
        jumpButton = input->pressed(JoystickButton::A) && Nedrysoft::Input::getInstance()->enabled(JoystickButton::A);
        leftButton = input->pressed(JoystickButton::Left);
        rightButton = input->pressed(JoystickButton::Right);
        downButton = input->pressed(JoystickButton::Down);
    }

    // 1. update ground speed with slope factor

    if (groundMode() != GroundMode::Ceiling) {
        float slopeFactor = SonicGroundSlopeRollingDownHill;

        float angleSin = sinf(degToRad(m_groundAngle));

        if (sign(angleSin) == sign(m_groundSpeed)) {
            slopeFactor = SonicGroundSlopeRollingUpHill;
        } 

        m_groundSpeed -= slopeFactor * angleSin;
    } else {
        if (sonicInState(m_currentState, SonicStateFlags::BallOnGround)) {
            modifyState(SonicStateFlags::Waiting);

            return;
        }
    }

    // 2. check for starting a jump

    if ((jumpButton) && (!sonicInState(m_currentState, SonicStateFlags::BallInTunnel))) {
        /**
         * now we transition to airborne state.
         */

        m_xSpeed -= SonicAirborneJumpForceCoefficient * sinf(degToRad(m_groundAngle));
        m_ySpeed -= SonicAirborneJumpForceCoefficient * cosf(degToRad(m_groundAngle));

        modifyState(Nedrysoft::SonicStateFlags::Jumping);

        //Nedrysoft::Input::getInstance()->setEnabled(JoystickButton::A, false);

        return;
    }

    // 3. update ground speed with controls

    if ((m_groundSpeed > 0.0f)) {
        if (leftButton) {
            m_groundSpeed -= SonicGroundRollDecelerationSpeed;
        }

        m_groundSpeed -= SonicGroundRollFrictionSpeed;

        if (m_groundSpeed <= 0) {
            m_groundSpeed = 0; //-0.5;

            if (!sonicInState(m_currentState, SonicStateFlags::BallInTunnel)) {
                setState(SonicStateFlags::WalkingRight);
            }
        } 
    } else if (m_groundSpeed < 0.0f) {
        if (rightButton) {
            m_groundSpeed += SonicGroundRollDecelerationSpeed;
        }

        m_groundSpeed += SonicGroundRollFrictionSpeed;

        if (m_groundSpeed >= 0) {
            m_groundSpeed = 0;// 0.5;

            if (!sonicInState(m_currentState, SonicStateFlags::BallInTunnel)) {
                setState(SonicStateFlags::WalkingLeft);
            }
        } 
    } 

    m_xSpeed = std::max(-SonicMaximumXSpeed, std::min(m_groundSpeed * cosf(degToRad(m_groundAngle)), SonicMaximumXSpeed));
    m_ySpeed = m_groundSpeed * -sinf(degToRad(m_groundAngle));

    m_groundSpeed = std::max(-SonicMaximumXSpeed, std::min(m_groundSpeed, SonicMaximumXSpeed));

    auto sensorAdjustment = Vector(m_xSpeed, 0);
    auto activatedSensor = SensorPosition::None;
    auto sensorDistance = 0.0f;

    // 4. check wall collisions

    if (checkWallSensors(tileMap, groundMode(), &activatedSensor, &sensorDistance, sensorAdjustment)) {
        if (activatedSensor == SensorPosition::Right) {
            if (sensorDistance < 0.0f) {
                if (!sonicInState(m_currentState, SonicStateFlags::BallInTunnel)) {
                    setState(SonicStateFlags::Waiting);

                    m_groundSpeed = 0;
                }
            }
        } else { 
            if (sensorDistance < 0.0f) {
                if (!sonicInState(m_currentState, SonicStateFlags::BallInTunnel)) {
                    m_groundSpeed = 0;

                    setState(SonicStateFlags::Waiting);
                }
            }
        }
    }

    // 5. check for screen boundaries and kill plane collisions

    // 6. update player position

    m_x += m_xSpeed;
    m_y += m_ySpeed;

    clipPosition(tileMap);

    // 7. check ground sensors

    Vector sensorOffset = {0.0f, 0.0f};

    float distance;
    float angle;

    SensorPosition activatedGroundSensors[] = {Nedrysoft::SensorPosition::None, Nedrysoft::SensorPosition::None, Nedrysoft::SensorPosition::None};

    if (checkGroundSensors(tileMap, activatedGroundSensors, &distance, &angle, sensorOffset)) {
        m_groundAngle = snapToGround(angle, m_groundAngle);

        move(0.0f, distance);
    } else {
        modifyState(SonicStateFlags::BallInAir);
    }

    if ((sonicInState(m_currentState, SonicStateFlags::BallInTunnel)) && (m_groundSpeed == 0.0f)) {
        m_groundSpeed = 2.0f;

        modifyState(SonicStateFlags::BallInTunnel);

        return;
    } 

    // 8. check for falling if ground speed is too slow

    if (!sonicInState(m_currentState, SonicStateFlags::BallInTunnel)) {
        if (sonicOnGround(m_currentState)) {
            if (!m_controlLockTimer) {
                if ((abs(m_groundSpeed) < 2.5) && ((groundMode() != GroundMode::Floor) || ((m_groundAngle >= 46.0f) && (m_groundAngle <= 315.0f)))) {
                    m_groundSpeed = 0.0f;

                    detachFromGround();

                    m_controlLockTimer = SonicFallingControlLockTimer;
                }
            } else {
                m_controlLockTimer--;
            }
        }
    }
}

auto Nedrysoft::Sonic::render(Camera *camera) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    if (m_invulnerabilityTimer) {
        if ((m_invulnerabilityTimer % 8) > 4) {
            return;
        }
    }

    SDL_Rect dest;

    Vector origin;

    assert(m_currentAnimation.current() != nullptr);

    auto texture = m_currentAnimation.texture(&dest.w, &dest.h, &origin);

    assert(texture != nullptr);

    dest.x = static_cast<int>(round(m_x) - origin.x() - camera->position().x());
    dest.y = static_cast<int>(round(m_y) - origin.y() - camera->position().y());

    int flipFlags = SDL_FLIP_NONE;

    if ((m_currentState & SonicStateFlags::Mirrored) == SonicStateFlags::Mirrored) {
        flipFlags ^= SDL_FLIP_HORIZONTAL;
    }

    SDL_Point rotationPoint;

    rotationPoint.x = dest.x;
    rotationPoint.y = dest.y;

    auto angle = m_groundAngle;

    if ((angle >= 45) && (angle < 90)) {
        angle = -45;
    } else if ((angle >= 90) && (angle < 135)) {
        angle = -90;
    } else if ((angle >= 135) && (angle < 180)) {
        angle = -135;
    } else if ((angle >= 180) && (angle < 225)) {
        angle = -180;
    } else if ((angle >= 225) && (angle < 270)) {
        angle = -225;
    } else if ((angle >= 270) && (angle < 315)) {
        angle = -270;
    } else {
        angle = 0;
    }

    if (flipFlags & SDL_FLIP_HORIZONTAL) {
        dest.x = static_cast<int>(round(m_x) - (static_cast<float>(dest.w) - origin.x()) - camera->position().x());
    }

    if (flipFlags & SDL_FLIP_VERTICAL) {
        dest.y = static_cast<int>(round(m_y) - (static_cast<float>(dest.h) - origin.y()) - camera->position().y());
    }

    SDL_RenderCopyEx(gameRenderer->renderer(), texture, nullptr, &dest, angle, nullptr, static_cast<SDL_RendererFlip>(flipFlags));
}

auto Nedrysoft::Sonic::drawSensor(float x, float y, int8_t red, uint8_t green, uint8_t blue, uint8_t alpha, Camera *camera) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    auto constexpr SensorRadius = 1;
    auto constexpr OutlineRadius = 2;
    
    SDL_Rect rect;

    rect.x = round(x - SensorRadius) - camera->position().x();
    rect.y = round(y - SensorRadius) - camera->position().y();
    rect.w = SensorRadius * 2;
    rect.h = SensorRadius * 2;

    SDL_Rect outerRect;

    outerRect.x = round(x - OutlineRadius) - camera->position().x();
    outerRect.y = round(y - OutlineRadius) - camera->position().y();
    outerRect.w = OutlineRadius * 2;
    outerRect.h = OutlineRadius * 2;

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0x00, 0x00, 0x00, 0xFF);

    SDL_RenderFillRect(gameRenderer->renderer(), &outerRect);

    SDL_SetRenderDrawColor(gameRenderer->renderer(), red, green, blue, alpha);
    
    SDL_RenderFillRect(gameRenderer->renderer(), &rect);
}

auto Nedrysoft::Sonic::renderDebug(Camera *camera) -> void {
    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    if (isDying()) {
        return;
    }

    float aX;
    float aY;
    float bX;
    float bY;
    float gX;
    float gY;

    float eX = NAN;
    float eY = NAN; 
    float fX = NAN;
    float fY = NAN; 

    Vector sensorOffset(0, (m_groundAngle == 0) ? 8.0f : 0.0f);

    switch(groundMode()) {
        case GroundMode::Floor: {
            eX = round(m_x - (SonicPushRadius + sensorOffset.x()));
            eY = round(m_y + sensorOffset.y());

            fX = round(m_x + (SonicPushRadius + sensorOffset.x()));
            fY = round(m_y + sensorOffset.y());

            aX = round(m_x - SonicPushRadius);
            aY = round(m_y + (height() / 2.0f));

            bX = round(m_x + (SonicPushRadius - 1));
            bY = round(m_y + (height() / 2.0f));

            gX = round(m_x);
            gY = round(m_y + (height() / 2.0f));

            break;
        }

        case GroundMode::RightWall: {
            eX = round(m_x + sensorOffset.y());
            eY = round(m_y + (SonicPushRadius + sensorOffset.x()));

            fX = round(m_x + sensorOffset.y());
            fY = round(m_y - (SonicPushRadius + sensorOffset.x()));

            aX = round(m_x + (height() / 2.0f));
            aY = round(m_y - SonicPushRadius);

            bX = round(m_x + (height() / 2.0f));
            bY = round(m_y + (SonicPushRadius - 1.0f));

            gX = round(m_x + (height() / 2.0f));
            gY = round(m_y);

            break;
        }

        case GroundMode::LeftWall: {
            eX = round(m_x + sensorOffset.y());
            eY = round(m_y - (SonicPushRadius + sensorOffset.x()));

            fX = round(m_x + sensorOffset.y());
            fY = round(m_y + (SonicPushRadius + sensorOffset.x()));

            aX = round(m_x - (height() / 2.0f));
            aY = round(m_y - SonicPushRadius);

            bX = round(m_x - (height() / 2.0f));
            bY = round(m_y + (SonicPushRadius - 1.0f));

            gX = round(m_x - (height() / 2.0f));
            gY = round(m_y);
            
            break;
        }

        case GroundMode::Ceiling: {
            aX = round(m_x - SonicPushRadius);
            aY = round(m_y - (height() / 2.0f));

            bX = round(m_x + (SonicPushRadius - 1.0f));
            bY = round(m_y - (height() / 2.0f));

            gX = round(m_x);
            gY = round(m_y - (height() / 2.0f));

            break;
        }

        case GroundMode::Undefined: {
            return;
        }   
    }

    drawSensor(m_x, m_y, 0xFF, 0xFF, 0x00, 0xFF, camera);

    drawSensor(aX, aY, 0x00, 0xFF, 0xFF, 0xFF, camera);
    drawSensor(bX, bY, 0xFF, 0x00, 0xFF, 0xFF, camera);

    if ((eX != NAN) && (eY != NAN)) {
        drawSensor(eX, eY, 0x00, 0xFF, 0xFF, 0xFF, camera);
    }

    if ((fX != NAN) && (fY != NAN)) {
        drawSensor(fX, fY, 0x00, 0xFF, 0x00, 0xFF, camera);
    }

    if (m_groundSpeed == 0) {
        drawSensor(gX, gY, 0x00, 0xAA, 0xAA, 0xFF, camera);
    }

    /*rect.x = round(m_x - SensorRadius - camera->position().x());
    rect.y = round(m_y - SensorRadius - camera->position().y());

    rect.w = SensorRadius * 2;
    rect.h = SensorRadius * 2;

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0xFF, 0x00, 0xFF);
    SDL_RenderFillRect(gameRenderer->renderer(), &rect);

    rect.x = aX - SensorRadius - camera->position().x();
    rect.y = aY - SensorRadius - camera->position().y();

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0x00, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(gameRenderer->renderer(), &rect);

    rect.x = bX - SensorRadius - camera->position().x();
    rect.y = bY - SensorRadius - camera->position().y();

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0x00, 0xFF, 0xFF);
    SDL_RenderFillRect(gameRenderer->renderer(), &rect);

    if ((eX != NAN) && (eY != NAN)) {
        rect.x = eX - SensorRadius - camera->position().x();
        rect.y = eY - SensorRadius - camera->position().y();

        SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0x00, 0x00, 0xFF);
        SDL_RenderFillRect(gameRenderer->renderer(), &rect);
    }

    if ((fX != NAN) && (fY != NAN)) {
        rect.x = fX - SensorRadius - camera->position().x();
        rect.y = fY - SensorRadius - camera->position().y();

        SDL_SetRenderDrawColor(gameRenderer->renderer(), 0x00, 0xFF, 0x00, 0xFF);
        SDL_RenderFillRect(gameRenderer->renderer(), &rect);
    }*/

    auto hitRect = hitBox();

    hitRect.x -= static_cast<int>(camera->position().x());
    hitRect.y -= static_cast<int>(camera->position().y());

    SDL_SetRenderDrawColor(gameRenderer->renderer(), 0xFF, 0xFF, 0xFF, 0x40);
    SDL_RenderFillRect(gameRenderer->renderer(), &hitRect);
}

auto Nedrysoft::Sonic::checkWallSensors(
        TileMap *tileMap,
        GroundMode groundMode,
        SensorPosition *activatedSensor,
        float *distance,
        const Vector &sensorOffset) -> bool {

    if ((m_groundAngle > 90) && (m_groundAngle < 270)) {
        *activatedSensor = SensorPosition::None;

        return false;
    }

    float fX, fY, eX, eY;

    *activatedSensor = SensorPosition::None;

    switch(groundMode) {
        case GroundMode::Floor: {
            eX = round(m_x - (SonicPushRadius + sensorOffset.x()));
            eY = round(m_y + sensorOffset.y());

            fX = round(m_x + (SonicPushRadius + sensorOffset.x()));
            fY = round(m_y + sensorOffset.y());

            if (m_groundSpeed > 0.0f) {
                *activatedSensor = SensorPosition::Right;

                return tileMap->findRight(Vector(fX, fY), distance);
            }

            *activatedSensor = SensorPosition::Left;

            return tileMap->findLeft(Vector(eX, eY), distance);
        }

        case GroundMode::RightWall: {
            eX = round(m_x + sensorOffset.y());
            eY = round(m_y + (SonicPushRadius + sensorOffset.x()));
            
            fX = round(m_x + sensorOffset.y());
            fY = round(m_y - (SonicPushRadius + sensorOffset.x()));

            if (m_groundSpeed > 0.0f) {
                *activatedSensor = SensorPosition::Right;

                return tileMap->findUp(Vector(fX, fY), distance);
            }

            *activatedSensor = SensorPosition::Left;

            return tileMap->findDown(Vector(eX, eY), distance);
        }

        case GroundMode::LeftWall: {
            eX = round(m_x + sensorOffset.y());
            eY = round(m_y - (SonicPushRadius + sensorOffset.x()));

            fX = round(m_x + sensorOffset.y());
            fY = round(m_y + (SonicPushRadius + sensorOffset.x()));

            if (m_groundSpeed > 0.0f) {
                *activatedSensor = SensorPosition::Right;

                return tileMap->findDown(Vector(fX, fY), distance);
            }

            *activatedSensor = SensorPosition::Left;

            return tileMap->findUp(Vector(eX, eY), distance);
        }

        case GroundMode::Ceiling: {
            return false;
        }

        case GroundMode::Undefined: {
            return false;
        }   
    }

    return false;
}

auto Nedrysoft::Sonic::checkGroundSensors(TileMap *tileMap, SensorPosition *activatedSensors, float *distance, float *groundAngle, const Vector &sensorOffset) -> bool {
    float distances[3];
    float angles[3];

    constexpr auto LeftSensor = 1;
    constexpr auto RightSensor = 2;
    constexpr auto BothSensors = 3;
    constexpr auto CentreSensor = 4;

    activatedSensors[0] = Nedrysoft::SensorPosition::None;
    activatedSensors[1] = Nedrysoft::SensorPosition::None;
    activatedSensors[2] = Nedrysoft::SensorPosition::None;

    int sensorsActivated = 0;

    switch(groundMode()) {
        case GroundMode::Floor: {
            sensorsActivated |= tileMap->findDown(
                Vector(round(m_x - SonicPushRadius), round(m_y + (height() / 2.0f))),
                &distances[0],
                &angles[0]
            ) ? LeftSensor : 0;

            sensorsActivated |= tileMap->findDown(
                Vector(round(m_x + (SonicPushRadius - 1)), round(m_y + (height() / 2.0f))),
                &distances[1],
                &angles[1]
            ) ? RightSensor : 0;

            if (m_groundSpeed == 0) {
                sensorsActivated |= tileMap->findDown(
                    Vector(round(m_x), round(m_y + (height() / 2.0f))),
                    &distances[2],
                    &angles[2]
                ) ? CentreSensor : 0;
            }

            break;
        }

        case GroundMode::RightWall: {
            sensorsActivated |= tileMap->findRight(
                Vector(round(m_x + (height() / 2.0f)), round(m_y - SonicPushRadius)),
                &distances[0],
                &angles[0]
            ) ? LeftSensor : 0;

            sensorsActivated |= tileMap->findRight(
                Vector(round(m_x + (height() / 2.0f)), round(m_y + (SonicPushRadius - 1.0f))),
                &distances[1],
                &angles[1]
            ) ? RightSensor : 0;

            if (m_groundSpeed == 0) {
                sensorsActivated |= tileMap->findRight(
                    Vector(round(m_x + (height() / 2.0f)), round(m_y)),
                    &distances[2],
                    &angles[2]
                ) ? CentreSensor : 0;
            }

            break;
        }

        case GroundMode::Ceiling: {
            sensorsActivated |= tileMap->findUp(
                Vector(round(m_x - SonicPushRadius), round(m_y - (height() / 2.0f))),
                &distances[0],
                &angles[0]
            ) ? LeftSensor : 0;

            sensorsActivated |= tileMap->findUp(
                Vector(round(m_x + (SonicPushRadius - 1.0f)), round(m_y - (height() / 2.0f))),
                &distances[1],
                &angles[1]
            ) ? RightSensor : 0;

            if (m_groundSpeed == 0) {
                sensorsActivated |= tileMap->findUp(
                    Vector(round(m_x), round(m_y - (height() / 2.0f))),
                    &distances[2],
                    &angles[2]
                ) ? CentreSensor : 0;
            }

            break;
        }

        case GroundMode::LeftWall: {
            sensorsActivated |= tileMap->findLeft(
                Vector(round(m_x - (height() / 2.0f)), round(m_y - SonicPushRadius)),
                &distances[0],
                &angles[0]
            ) ? LeftSensor : 0;

            sensorsActivated |= tileMap->findLeft(
                Vector(round(m_x - (height() / 2.0f)), round(m_y + (SonicPushRadius - 1.0f))),
                &distances[1],
                &angles[1]
            ) ? RightSensor : 0;

            if (m_groundSpeed == 0) {
                sensorsActivated |= tileMap->findLeft(
                    Vector(round(m_x - (height() / 2.0f)), round(m_y)),
                    &distances[2],
                    &angles[2]
                ) ? CentreSensor : 0;    
            }

            break;
        }

        default: {
            //assert(false);

            return false;
        }
    }

    switch(sensorsActivated & (BothSensors)) {
        case LeftSensor: {
            *distance = round(distances[0]);
            *groundAngle = round(angles[0]);

            activatedSensors[0] = Nedrysoft::SensorPosition::Left;

            if (sensorsActivated & CentreSensor) {
                activatedSensors[1] = Nedrysoft::SensorPosition::Centre;
            }

            return true;
        }

        case RightSensor: {
            *distance = round(distances[1]);
            *groundAngle = round(angles[1]);

            activatedSensors[0] = Nedrysoft::SensorPosition::Right;

            if (sensorsActivated & CentreSensor) {
                activatedSensors[1] = Nedrysoft::SensorPosition::Centre;
            }

            return true;
        }

        case BothSensors: {
            if (distances[0] < distances[1]) {
                *distance = round(distances[0]);
                *groundAngle = round(angles[0]);

                activatedSensors[0] = Nedrysoft::SensorPosition::Left;
                activatedSensors[1] = Nedrysoft::SensorPosition::Right;
            } else {
                *distance = round(distances[1]);
                *groundAngle = round(angles[1]);
                
                activatedSensors[0] = Nedrysoft::SensorPosition::Right;
                activatedSensors[1] = Nedrysoft::SensorPosition::Left;
            }

            return true;
        }

        default: {
            return false;
        }
    }

    if (distances[0] < distances[1]) {
        *distance = round(distances[0]);
        *groundAngle = round(angles[0]);
    } else {
        *distance = round(distances[1]);
        *groundAngle = round(angles[1]);
    }

    return false;
}

auto Nedrysoft::Sonic::checkCeilingSensors(TileMap *tileMap, GroundMode groundMode) -> void {
    N_UNUSED(groundMode)

    auto constexpr LeftSensor = 1;
    auto constexpr RightSensor = 2;

    float distance[2];

    if (sonicInAir(m_currentState)) {
        return;
    }

    int sensorsActivated = 0.0f;

    if (m_ySpeed < 0.0f) {
        sensorsActivated |= tileMap->findUp(
                rect().topLeft(),
                &distance[0]
        ) ? LeftSensor : 0;

        sensorsActivated |= tileMap->findUp(
                rect().topRight().add(-1.0f, 0.0f),
                &distance[1]
        ) ? RightSensor : 0;
    }

    switch(sensorsActivated) {
        case LeftSensor: {
            if (distance[0] < 0.0f) {
                move(0.0f, -distance[0]);
            }

            break;
        }

        case RightSensor: {
            if (distance[1] < 0.0f) {
                move(0.0f, -distance[1]);
            }

            break;
        }

        case LeftSensor | RightSensor: {
            if (std::min<float>(distance[0], distance[1]) < 0.0f) {
                move(0.0f, -std::min<float>(distance[0], distance[1]));
            }

            break;
        }

        default: {
            break;
        }
    }
}

auto Nedrysoft::Sonic::groundMode() const -> GroundMode {
    GroundMode groundMode = GroundMode::Undefined;

    if ((m_groundAngle >= 315.0f) || (m_groundAngle < 45.0f)) {
        groundMode = GroundMode::Floor;
    } else if ((m_groundAngle >= 45.0f) && (m_groundAngle < 135.0f)) {
        groundMode = GroundMode::RightWall;
    } else if ((m_groundAngle >= 135.0f) && (m_groundAngle < 226.0f)) {
        groundMode = GroundMode::Ceiling;
    } else if ((m_groundAngle >= 226.0f) && (m_groundAngle < 315.0f)) {
        groundMode = GroundMode::LeftWall;
    }

    return groundMode;
}

auto Nedrysoft::Sonic::updatePositionDebug(TileMap *tileMap) -> void {
    N_UNUSED(tileMap)

    auto input = Input::getInstance();

    if (input->pressed(JoystickButton::Left)) {
        m_groundSpeed = -4.0f;
    } else if (input->pressed(JoystickButton::Right)) {
        m_groundSpeed = 4.0f;
    } else {
        m_groundSpeed = 0.0f;
    }

    m_xSpeed = m_groundSpeed;

    if (input->pressed(JoystickButton::Up)) {
        m_ySpeed = -4.0f;
    } else if (input->pressed(JoystickButton::Down)) {
        m_ySpeed = 4.0f;
    } else {
        m_ySpeed = 0.0f;
    }

    m_x += m_xSpeed;
    m_y += m_ySpeed;

    if (m_x < (width() / 2.0f)) {
        m_x = (width() / 2.0f);
    }

    if (m_x > static_cast<float>(tileMap->widthInPixels()) - (width() / 2.0f)) {
        m_x = static_cast<float>(tileMap->widthInPixels()) - (width() / 2.0f);
    }

    if (m_y < (height() / 2.0f)) {
        m_y = (height() / 2.0f);
    }

    if (m_y > static_cast<float>(tileMap->heightInPixels()) - (height() / 2.0f)) {
        m_y = static_cast<float>(tileMap->heightInPixels()) - (height() / 2.0f);
    }
}

auto Nedrysoft::Sonic::groundSpeed() const -> float {
    return m_groundSpeed;
}

auto Nedrysoft::Sonic::maxGroundSpeed() const ->  float {
    return m_maxGroundSpeed;
}

auto Nedrysoft::Sonic::horizontalSpeed() const -> float {
    return m_xSpeed;
}

auto Nedrysoft::Sonic::verticalSpeed() const -> float {
    return m_ySpeed;
}

auto Nedrysoft::Sonic::position() -> Vector {
    return rect().bottomLeft();
}

auto Nedrysoft::Sonic::clipPosition(TileMap *tileMap) -> void {
    if (m_x < (width() / 2.0f)) {
        m_x = (width() / 2.0f);
        m_groundSpeed = 0.0f;
        m_xSpeed = 0.0f;
    }

    if (m_x > static_cast<float>(tileMap->widthInPixels()) - (width() / 2.0f)) {
        m_x = static_cast<float>(tileMap->widthInPixels()) - (width() / 2.0f);
        m_groundSpeed = 0.0f;
        m_xSpeed = 0.0f;
    }

    if (m_y < (height() / 2.0f)) {
        m_y = (height() / 2.0f);
        m_groundSpeed = 0.0f;
        m_ySpeed = 0.0f;
    }

    if (m_y > static_cast<float>(tileMap->heightInPixels()) - (height() / 2.0f)) {
        m_y = static_cast<float>(tileMap->heightInPixels()) - (height() / 2.0f);
        m_groundSpeed = 0.0f;
        m_ySpeed = 0.0f;
    }
}

auto Nedrysoft::Sonic::width() -> float {
    return m_currentAnimation.width();
}

auto Nedrysoft::Sonic::height() -> float {
    return m_currentAnimation.height();
}

auto Nedrysoft::Sonic::stateString(SonicStateFlags state) -> std::string {
    std::string stateString;

    switch(sonicState(state)) {
        case SonicStateFlags::BallOnGround: {
            stateString = "Ball on ground";
            break;
        }
        case SonicStateFlags::Jumping: {
            stateString = "Jumping";
            break;
        }
        case SonicStateFlags::Pushing: {
            stateString = "Pushing";
            break;
        }
        case SonicStateFlags::StandingOnObject: {
            stateString = "Standing on object";
            break;
        }

        case SonicStateFlags::StandingOnGround: {
            stateString = "Standing on ground";
            break;
        }

        case SonicStateFlags::Waiting: {
            stateString = "Waiting";
            break;
        }

        case SonicStateFlags::Falling: {
            stateString = "Falling";
            break;
        }

        case SonicStateFlags::Walking: {
            stateString = "Walking";
            break;
        }

        case SonicStateFlags::Stopping: {
            stateString = "Stopping";
            break;
        }

        case SonicStateFlags::Running: {
            stateString = "Running";
            break;
        }

        case SonicStateFlags::BallInTunnel: {
            stateString = "BallInTunnel";
            break;
        }

        case SonicStateFlags::BallInAir: {
            stateString = "BallInAir";
            break;
        }

        case SonicStateFlags::Rocket: {
            stateString = "Rocket";
            break;
        }

        case SonicStateFlags::Dead: {
            stateString = "Dead";
            break;
        }

        default: {
            stateString = toHexString(sonicState(state), 8);
            break;
        }

    }
    return stateString + ((sonicFacingLeft(state) ? " facing left" : " facing right"));
}

auto Nedrysoft::Sonic::setState(SonicStateFlags state) -> void {
    /**
     * first up, if nothings changed....nothing has changed, so nothing to change!
     */
    if (state == m_currentState) {
        return;
    }

    /**
     * here we check if the state is the same, if it is then it's only the direction that has changed, so we can
     * just set the mirroring on the current animation to reflect the change.
     */

    if (sonicState(state) == sonicState(m_currentState)) {
        m_currentState = state;

        return;
    }

    m_currentState = state;

    switch(sonicState(m_currentState)) {
        case SonicStateFlags::Running: {
            m_animations->start({"run", "run2", "run3", "run4"}, &m_currentAnimation, [&](const std::string &, AnimationPlayer *) {
                return floor(std::max<float>(0.0f, 8.0f - abs(m_groundSpeed)));
            });

            break;
        }

        case SonicStateFlags::Walking: {
            m_animations->start({"walk", "walk2", "walk3", "walk4"}, &m_currentAnimation, [&](const std::string &, AnimationPlayer *) {
                return floor(std::max<float>(0.0f, 8.0f - abs(m_groundSpeed)));
            });

            break;
        }

        case SonicStateFlags::Falling: {
            m_animations->start({"walk", "walk2", "walk3", "walk4"}, &m_currentAnimation, [&](const std::string &, AnimationPlayer *) {
                return 24.0f;
            });

            break;
        }

        case SonicStateFlags::Jumping:
        case SonicStateFlags::BallOnGround:
        case SonicStateFlags::BallInTunnel:
        case SonicStateFlags::BallInAir: {
            m_animations->start("roll", &m_currentAnimation, [&](const std::string &, AnimationPlayer *) {
                return floor(std::max<float>(0.0f, 4.0f - abs(m_groundSpeed)));
            });

            break;
        }

        case SonicStateFlags::Pushing: {
            m_animations->start("push", &m_currentAnimation, [&](const std::string &, AnimationPlayer *) {
                return floor(std::max<float>(0.0f, 8.0f - abs(m_groundSpeed) * 4.0f));
            });

            break;
        }

        case SonicStateFlags::StandingOnObject:
        case SonicStateFlags::Waiting:
        case SonicStateFlags::StandingOnGround: {
            m_animations->start("wait", &m_currentAnimation);

            break;
        }

        case SonicStateFlags::Stopping: {
            m_animations->start("stop", &m_currentAnimation);

            break;
        }

        case SonicStateFlags::Hurt: {
            m_animations->start("hurt", &m_currentAnimation);

            break;
        }

        case SonicStateFlags::Dead:
        case SonicStateFlags::Dying: {
            m_animations->start("death", &m_currentAnimation);

            break;
        }

        case SonicStateFlags::Rocket: {
            m_animations->start("spring", &m_currentAnimation);

            break;
        }

        case SonicStateFlags::Balancing: {
            m_animations->start("balance", &m_currentAnimation);

            break;
        }

        default: {
            assert(false);

            break;
        }
    }
}

auto Nedrysoft::Sonic::modifyState(SonicStateFlags newState) -> void {
    auto newCompositeState = newState;

    if ((m_currentState & Nedrysoft::SonicStateFlags::Mirrored) == Nedrysoft::SonicStateFlags::Mirrored) {
        newCompositeState = newCompositeState | Nedrysoft::SonicStateFlags::Mirrored;
    }

    setState(newCompositeState);
}

auto Nedrysoft::Sonic::groundModeString() -> std::string {
    switch(groundMode()) {
        case GroundMode::Floor: {
            return "Floor";
        }

        case GroundMode::RightWall: {
            return "Right wall";
        }

        case GroundMode::Ceiling: {
            return "Ceiling";
        }

        case GroundMode::LeftWall: {
            return "Left wall";
        }

        default: {
            break;
        }
    }

    return "Unknown";
}

auto Nedrysoft::Sonic::move(float deltaX, float deltaY) -> void {
    /**
     * this function moves sonic taking into account the current ground mode, as he transitions from floor to
     * right wall, ceiling and left wall the X & Y gets swapped, so when applying a delta we need to take the
     * current floor mode into account otherwise sonic will not move to the correct position
     */

    switch(groundMode()) {
        case GroundMode::Floor: {
            m_x += deltaX;
            m_y += deltaY;

            return;
        }

        case GroundMode::RightWall: {
            m_y += deltaX;
            m_x += deltaY;

            return;
        }

        case GroundMode::Ceiling: {
            m_x -= deltaX;
            m_y -= deltaY;

            return;
        }

        case GroundMode::LeftWall: {
            m_y -= deltaX;
            m_x -= deltaY;

            return;
        }

        case GroundMode::Undefined: {
            return;
        }
    }
}

auto Nedrysoft::Sonic::onGround() const -> bool {
    return sonicOnGround(m_currentState);
}

auto Nedrysoft::Sonic::detachFromGround() -> void {
    modifyState(SonicStateFlags::Falling);
}

auto Nedrysoft::Sonic::groundSteepness() const -> GroundSteepness {
    if ( ((m_groundAngle >= 0.0f) && (m_groundAngle <= 23.0f)) || ((m_groundAngle >= 339.0f) && (m_groundAngle <= 360.0f))) {
        return GroundSteepness::Shallow;
    } else if ( ((m_groundAngle >= 24.0f) && (m_groundAngle <= 45.0f)) || ((m_groundAngle >= 316.0f) && (m_groundAngle <= 338.0f))) {
        return GroundSteepness::HalfSteep;
    } else if ( ((m_groundAngle >= 46.0f) && (m_groundAngle <= 90.0f)) || ((m_groundAngle >= 271.0f) && (m_groundAngle <= 315.0f))) {
        return GroundSteepness::FullSteep;
    }

    return OutOfRange;
}

auto Nedrysoft::Sonic::movingMostlyHorizontal() const -> bool {
    return fabs(m_xSpeed) > fabs(m_ySpeed);
}

auto Nedrysoft::Sonic::hitBox() -> SDL_Rect {
    SDL_Rect rect = {
        .x = 0,
        .y = 0,
        .w = 0,
        .h = 0
    };

    auto SonicHitboxVerticalRadius = (height() / 2);

    switch(groundMode()) {
        case GroundMode::Floor: {
            rect.x = static_cast<int>(round(m_x) - SonicPushRadius);
            rect.y = static_cast<int>(round(m_y) - SonicHitboxVerticalRadius);

            rect.w = static_cast<int>(SonicPushRadius * 2);
            rect.h = static_cast<int>(SonicHitboxVerticalRadius * 2);

            break;
        }

        case GroundMode::RightWall: {
            rect.x = static_cast<int>(round(m_x) - SonicHitboxVerticalRadius);
            rect.y = static_cast<int>(round(m_y) - SonicPushRadius);

            rect.h = static_cast<int>(SonicPushRadius * 2);
            rect.w = static_cast<int>(SonicHitboxVerticalRadius * 2);

            break;
        }

        case GroundMode::Ceiling: {
            rect.x = static_cast<int>(round(m_x) - SonicPushRadius);
            rect.y =  static_cast<int>(round(m_y) - SonicHitboxVerticalRadius);

            rect.w = static_cast<int>(SonicPushRadius * 2);
            rect.h = static_cast<int>(SonicHitboxVerticalRadius * 2);

            break;
        }

        case GroundMode::LeftWall: {
            rect.x = static_cast<int>(round(m_x) - SonicHitboxVerticalRadius);
            rect.y = static_cast<int>(round(m_y) - SonicPushRadius);

            rect.h = static_cast<int>(SonicPushRadius * 2);
            rect.w = static_cast<int>(SonicHitboxVerticalRadius * 2);

            break;
        }

        default:
            break;
    }

    return rect;
}

auto Nedrysoft::Sonic::setStandingOnObject(Object *object) -> void {
    if (isDying()) {
        return;
    }

    if (isHurt() && (m_ySpeed < 0)) {
        return;
    }

    m_standingOnObject = object;
    m_groundAngle = 0;

    reattachToGround(0.0f, 0.0f);
}

auto Nedrysoft::Sonic::standingOnObject() -> Object * {
    return m_standingOnObject;
}

auto Nedrysoft::Sonic::reattachToGround(int dx, int dy) -> void {
    auto steepness = groundSteepness();

    switch(groundSteepness()) {
        case Shallow: {
            m_groundSpeed = m_xSpeed;

            break;
        }

        case HalfSteep: {
            if (movingMostlyHorizontal()) {
                 m_groundSpeed = m_xSpeed;
            } else {
                m_groundSpeed = m_ySpeed * 0.5f * -sign(sin(degToRad(m_groundAngle)));
            }

            break;
        }

        case FullSteep: {
            if (movingMostlyHorizontal()) {
                m_groundSpeed = m_xSpeed;
            } else {
                m_groundSpeed = m_ySpeed * -sign(sin(degToRad(m_groundAngle)));
            }

            break;
        }

        case OutOfRange: {
            break;
        }
    }

    move(static_cast<float>(dx), static_cast<float>(dy));

    if (m_groundSpeed == 0.0f) {
        modifyState(Nedrysoft::SonicStateFlags::Waiting);
    } else if (std::round(abs(m_groundSpeed)) >= SonicGroundSpeedMax) {
        modifyState(Nedrysoft::SonicStateFlags::Running);
    } else {
        modifyState(Nedrysoft::SonicStateFlags::Walking);
    }
}

auto Nedrysoft::Sonic::isJumpingUp() -> bool {
    return sonicInState(m_currentState, SonicStateFlags::Jumping) && (m_ySpeed < 0);
}

auto Nedrysoft::Sonic::isJumpingDown() -> bool {
    return sonicInState(m_currentState, SonicStateFlags::Jumping) && (m_ySpeed > 0);
}

auto Nedrysoft::Sonic::setObjectCollidedRight(int collisionX, bool shouldPush) -> void {
    auto rightButton = Nedrysoft::Input::getInstance()->pressed(Nedrysoft::JoystickButton::Right);
    auto leftButton = Nedrysoft::Input::getInstance()->pressed(Nedrysoft::JoystickButton::Left);

    m_groundSpeed = 0.0f;
    m_xSpeed = 0.0f;
    m_x = static_cast<float>(collisionX);

    if (isDying()) {
        return;
    }

    if (isJumpingUp()) {
        return;
    }

    if (shouldPush) {
        if (rightButton) {
            modifyState(Nedrysoft::SonicStateFlags::Pushing);
        } else if (!leftButton) {
            modifyState(Nedrysoft::SonicStateFlags::Waiting);
        }
    } else {
        modifyState(Nedrysoft::SonicStateFlags::Falling);
    }
}

auto Nedrysoft::Sonic::setObjectCollidedLeft(int collisionX, bool shouldPush) -> void {
    auto leftButton = Nedrysoft::Input::getInstance()->pressed(Nedrysoft::JoystickButton::Left);
    auto rightButton = Nedrysoft::Input::getInstance()->pressed(Nedrysoft::JoystickButton::Right);

    m_groundSpeed = 0.0f;
    m_xSpeed = 0.0f;
    m_x = static_cast<float>(collisionX);

    if (isDying()) {
        return;
    }

    if (isJumpingUp()) {
        return;
    }

    if (shouldPush) {
        if (leftButton) {
            modifyState(Nedrysoft::SonicStateFlags::Pushing);
        } else if (!rightButton) {
            modifyState(Nedrysoft::SonicStateFlags::Waiting);
        }
    } else {
        modifyState(Nedrysoft::SonicStateFlags::Falling);

    }
}

auto Nedrysoft::Sonic::setObjectCollidedTop(int distance) -> void {
    m_ySpeed = -m_ySpeed;
}

auto Nedrysoft::Sonic::currentState() -> std::string {
    return stateString(m_currentState);
}

auto Nedrysoft::Sonic::centreOffset(int *x, int *y) -> void {
    Vector origin;
    int width, height;

    m_currentAnimation.texture(&width, &height, &origin);

    *x = static_cast<int>(origin.x());
    *y = static_cast<int>(origin.y());
}

auto Nedrysoft::Sonic::setJumping(float jumpForceY) -> void {
    m_ySpeed = -jumpForceY;

    modifyState(Nedrysoft::SonicStateFlags::Jumping);

    Nedrysoft::Audio::getInstance()->playSample(0, Nedrysoft::SoundId::Jump);

    m_standingOnObject = nullptr;
}

auto Nedrysoft::Sonic::setControlLock(int time) -> void {
    m_controlLockTimer = time;
}

auto Nedrysoft::Sonic::checkStateValid() -> void {
    if ( (m_currentState == SonicStateFlags::BallOnGroundLeft) ||
         (m_currentState == SonicStateFlags::BallOnGroundRight) ||
         (m_currentState == SonicStateFlags::JumpingLeft) ||
         (m_currentState == SonicStateFlags::JumpingRight) ||
         (m_currentState == SonicStateFlags::PushingLeft) ||
         (m_currentState == SonicStateFlags::PushingRight) ||
         (m_currentState == SonicStateFlags::StandingOnObjectLeft) ||
         (m_currentState == SonicStateFlags::StandingOnObjectRight) ||
         (m_currentState == SonicStateFlags::WaitingLeft) ||
         (m_currentState == SonicStateFlags::WaitingRight) ||
         (m_currentState == SonicStateFlags::FallingLeft) ||
         (m_currentState == SonicStateFlags::FallingRight) ||
         (m_currentState == SonicStateFlags::WalkingLeft) ||
         (m_currentState == SonicStateFlags::WalkingRight) ||
         (m_currentState == SonicStateFlags::RunningLeft) ||
         (m_currentState == SonicStateFlags::RunningRight) ||
         (m_currentState == SonicStateFlags::StoppingLeft) ||
         (m_currentState == SonicStateFlags::StoppingRight) ||
         (m_currentState == SonicStateFlags::BalancingLeft) ||
         (m_currentState == SonicStateFlags::BalancingRight) ||
         (m_currentState == SonicStateFlags::BallInTunnelLeft) ||
         (m_currentState == SonicStateFlags::BallInTunnelRight) ||
         (m_currentState == SonicStateFlags::BallInAirLeft) ||
         (m_currentState == SonicStateFlags::BallInAirRight) ) {
        return;
    }

    assert(false);
}

auto Nedrysoft::Sonic::scatterRings() -> void {
    float speed = ScatteredRingsOuterVelocity;
    float ringAngle = ScatteredRingAngle;

    for (int i = 0; i < std::min(ScatteredRingsMaximum, m_rings); i++) {
        float xSpeed = (cos(degToRad(ringAngle)) * speed) * (i & 1 ? -1.0f : 1.0f);
        float ySpeed = -sin(degToRad(ringAngle)) * speed;

        auto ring = new ScatteredRing(m_x, m_y, xSpeed, ySpeed);

        ObjectsManager::getInstance()->addDynamicObject(ring);

        if (i & 1) {
            ringAngle += ScatteredRingAngleSpacing;
        }

        if (i == (ScatteredRingsMaximum / 2) - 1) {
            speed = ScatteredRingsInnerVelocity / 2.0f;

            ringAngle = ScatteredRingAngle;
        }
    }

    Nedrysoft::Audio::getInstance()->playSample(1, Nedrysoft::SoundId::RingLoss);

    m_rings = 0;
}

auto Nedrysoft::Sonic::invulnerability() const -> int {
    return m_invulnerabilityTimer;
}

auto Nedrysoft::Sonic::hurt(float yDistance, Vector objectPosition, bool overrideInvulnerability) -> void {
    bool invulerability = m_invulnerabilityTimer != 0;

    if (overrideInvulnerability) {
        invulerability = false;
    }

    if ((m_rings == 0) && (!invulerability))  {
        kill();

        return;
    }

    if ((isHurt()) || (isDying()) || (invulerability)) {
        return;
    }

    modifyState(SonicStateFlags::Hurt);

    m_y += yDistance;

    m_ySpeed = -4;

    m_xSpeed = sign(m_x - objectPosition.x()) * 2.0f;

    m_xSpeed = (m_xSpeed == 0) ? 1 : m_xSpeed;

    scatterRings();

    m_invulnerabilityTimer = HurtInvulnerabilityFrames;

    m_standingOnObject = nullptr;
}

auto Nedrysoft::Sonic::kill() -> void {
    if (isDying()) {
        return;
    }

    modifyState(SonicStateFlags::Dying);

    m_xSpeed = 0;
    m_ySpeed = -7;

    m_standingOnObject = nullptr;
    m_dyingTimer = -1;

    m_animations->start("death", &m_currentAnimation);
}

auto Nedrysoft::Sonic::updateHurtState(TileMap *tileMap, Camera *camera) -> void {
    N_UNUSED(camera);

    SensorPosition activatedSensor;
    Vector sensorAdjustment;

    m_hurtTimer--;

    m_ySpeed += SonicAirborneHurtGravityCoefficient;

    m_y += m_ySpeed;

    clipPosition(tileMap);

    auto groundAngle = 0.0f;
    auto sensorDistance = 0.0f;

    if (!m_standingOnObject) {
        if (checkGroundSensors(tileMap, &activatedSensor, &sensorDistance, &groundAngle, sensorAdjustment)) {
            if (m_ySpeed >= 0.0f) {
                if (sensorDistance <= 0.0f) {
                    auto currentFloorHeight = m_y + sensorDistance;

                    reattachToGround(0, static_cast<int>(sensorDistance));

                    m_xSpeed = 0;
                }
            }
        }
    }
}

auto Nedrysoft::Sonic::updateDyingState(TileMap *tileMap, Camera *camera) -> void {
    N_UNUSED(camera);

    m_ySpeed += SonicAirborneGravityCoefficient;

    m_y += m_ySpeed;

    int width;
    int height;
    Vector origin;

    m_currentAnimation.texture(&width, &height, &origin);

    auto gameRenderer = Nedrysoft::GameRenderer::getInstance();

    SDL_Rect objectRect;
    SDL_Rect screenRect;
    SDL_Rect intersectRect;

    objectRect.x = m_x - (static_cast<float>(width) / 2.0f);
    objectRect.y = m_y - (static_cast<float>(height) / 2.0f);
    objectRect.w = width;
    objectRect.h = height;

    screenRect.x = camera->position().x();
    screenRect.y = camera->position().y();
    screenRect.w = gameRenderer->viewportWidth();
    screenRect.h = gameRenderer->viewportHeight();

    if (!SDL_IntersectRect(&objectRect, &screenRect, &intersectRect)) {
        if (m_dyingTimer == -1) {
            m_dyingTimer = 60;
        } else if (m_dyingTimer > 0) {
            m_dyingTimer--;
        } else {
            m_dyingTimer = -1;

            setState(SonicStateFlags::Dead);
        }
   }
}

auto Nedrysoft::Sonic::isDying() -> bool {
    return sonicInState(m_currentState, SonicStateFlags::Dying);
}

auto Nedrysoft::Sonic::isDead() -> bool {
    return sonicInState(m_currentState, SonicStateFlags::Dead);
}

auto Nedrysoft::Sonic::isHurt() -> bool {
    return sonicInState(m_currentState, SonicStateFlags::Hurt);
}

auto Nedrysoft::Sonic::addRings(int rings) -> void {
    m_rings += rings;
}

auto Nedrysoft::Sonic::rings() const -> int {
    return m_rings;
}

auto Nedrysoft::Sonic::addPoints(int points) -> void {
    m_points += points;
}

auto Nedrysoft::Sonic::points() const -> int {
    return m_points;
}

auto Nedrysoft::Sonic::addLives(int lives) -> void {
    m_lives += lives;
}

auto Nedrysoft::Sonic::lives() const -> int {
    return m_lives;
}

auto Nedrysoft::Sonic::attacking() const -> bool {
    return sonicInState(m_currentState, SonicStateFlags::BallOnGround) ||
           sonicInState(m_currentState, SonicStateFlags::BallInAir) ||
           sonicInState(m_currentState, SonicStateFlags::Jumping);
}

auto Nedrysoft::Sonic::rebound(ReboundMode mode, Vector objectPosition) -> void {
    if (sonicOnGround(m_currentState)) {
        return;
    }

    if (mode == ReboundMode::Monitor) {
        if (m_ySpeed > 0) {
            m_ySpeed *= -1;

            return;
        } else if (m_ySpeed == 0) {
            return;
        }

        if (m_y < objectPosition.y()) {
            //m_x += 16 - (m_x - objectPosition.x());

            if (objectPosition.x() >= m_x) {
                auto diff = 16 - (objectPosition.x() - m_x);

                m_x += diff;
            } else {
                auto diff = 16 - (m_x - objectPosition.x());

                m_x -= diff;
            }
        }

        return;
    }

    if ( (m_y > objectPosition.y()) || (m_ySpeed < 0) ) {
        m_ySpeed -= 1 * sign(m_ySpeed);
    } else if (m_ySpeed > 0) {
        if (m_y < objectPosition.y()) {
            m_ySpeed *= -1;
        }
    }
}

auto Nedrysoft::Sonic::rocket(float xDistance, float yDistance, float xSpeed, float ySpeed, int controlLockFrames) -> void {
    m_y += yDistance;
    m_x += xDistance;

    if (m_ySpeed != 0x0f) {
        m_ySpeed = ySpeed;
    }

    if (m_xSpeed != 0.0f) {
        m_xSpeed = xSpeed;
    }

    m_controlLockTimer = controlLockFrames;

    modifyState(SonicStateFlags::Rocket);
}
