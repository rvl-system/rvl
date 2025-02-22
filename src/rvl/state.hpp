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

void stateLoop();

uint32_t getAnimationClock();
void setAnimationClock(uint32_t newClock);

uint8_t getDeviceId();

uint8_t getChannel();
void setChannel(uint8_t channel);

DeviceMode getDeviceMode();
void setDeviceMode(DeviceMode newDeviceMode);

RVLWaveSettings* getWaveSettings();
void setWaveSettings(RVLWaveSettings* newWaveSettings);

bool getPowerState();
void setPowerState(bool newPowerState);

uint8_t getBrightness();
void setBrightness(uint8_t newBrightness);

bool getRemoteBrightnessState();
void setRemoteBrightnessState(bool newRemoteBrightness);

bool getSynchronizationState();
void setSynchronizationState(bool synchronized);

bool isNetworkConnected();

} // namespace rvl

#endif // RVL_STATE_H_
