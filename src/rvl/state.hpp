/*
Copyright (c) Bryan Hughes <bryan@nebri.us>

This file is part of RVL.

RVL is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RVL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RVL.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RVL_STATE_H_
#define RVL_STATE_H_

#include "./rvl.hpp"
#include "./wave.hpp"
#include <stdint.h>

namespace rvl {

enum class DeviceMode { Controller, Receiver };

enum class AnimationType { Off, Wave };

uint32_t getAnimationClock();
void adjustAnimationClock(int32_t delta);
uint32_t toAnimationClock(uint32_t localTime);

uint8_t getDeviceId();

uint8_t getChannel();
void setChannel(uint8_t channel);

DeviceMode getDeviceMode();
void setDeviceMode(DeviceMode newDeviceMode);

void lockState();
void freeState();

AnimationType getAnimationType();
void setOff();
RVLWaveSettings* getWaveSettings();
void setWaveSettings(RVLWaveSettings* newWaveSettings);

uint8_t getBrightness();
void setBrightness(uint8_t newBrightness);

// State inputs
bool getLinkUpState();
void setLinkUpState(bool linkUp);
void setDeviceId(uint8_t deviceId);
bool getClockSyncedState();
void setClockSyncedState(bool clockSynced);
void setControllerActiveState(bool controllerActive);

// Indicates whether or not we can communicate with other nodes
bool isConnected();

// Indicates whether or not we're safe to render LEDs
bool isReadyToRender();

// Indicates whether or not the transport mechanism is available, e.g. whether
// or not we're connected to the WiFi AP
bool isLinkUp();

} // namespace rvl

#endif // RVL_STATE_H_
