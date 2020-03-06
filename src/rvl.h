/*
Copyright (c) Bryan Hughes <bryan@nebri.us>

This file is part of RVL Arduino.

RVL Arduino is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RVL Arduino is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RVL Arduino.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RVL_H_
#define RVL_H_

#include <Arduino.h>

#include "./rvl/logging.h"
#include "./rvl/event.h"
#include "./rvl/state.h"

// TODO(nebrius): remove rvl:: prefixes once everything is in the same namespace

class RVLTransportInterface {
 public:
  virtual void beginWrite(uint8_t destination) = 0;
  virtual void write8(uint8_t data) = 0;
  virtual void write16(uint16_t data) = 0;
  virtual void write32(uint32_t data) = 0;
  virtual void write(uint8_t* data, uint16_t length) = 0;
  virtual void endWrite() = 0;

  virtual uint16_t parsePacket() = 0;
  virtual uint8_t read8() = 0;
  virtual uint16_t read16() = 0;
  virtual uint32_t read32() = 0;
  virtual void read(uint8_t* buffer, uint16_t length) = 0;

  virtual bool isConnected() = 0;
  virtual bool isNetworkAvailable() = 0;
  virtual uint16_t getDeviceId() = 0;
};

class RVLPlatformInterface {
 public:
  virtual void onWaveSettingsUpdated();
  virtual void onDeviceModeUpdated();
  virtual void onClockOffsetUpdated();
  virtual void onChannelUpdated();
  virtual void onPowerStateUpdated();
  virtual void onBrightnessUpdated();
  virtual void onSynchronizationStateUpdated();
};

void RVLMessagingInit(
  RVLPlatformInterface* platform,
  RVLTransportInterface* transport);

void RVLMessagingLoop();

#endif  // RVL_H_
