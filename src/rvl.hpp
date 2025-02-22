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

#ifndef RVL_H_
#define RVL_H_

#include <stdint.h>

#include "./rvl/event.hpp"
#include "./rvl/logging.hpp"
#include "./rvl/state.hpp"

#define EVENT_WAVE_SETTINGS_UPDATED 1
#define EVENT_DEVICE_MODE_UPDATED 2
#define EVENT_ANIMATION_CLOCK_UPDATED 3
#define EVENT_CHANNEL_UPDATED 4
#define EVENT_POWER_STATE_UPDATED 5
#define EVENT_BRIGHTNESS_UPDATED 6
#define EVENT_SYNCHRONIZATION_STATE_UPDATED 7
#define EVENT_CONNECTION_STATE_CHANGED 8
#define EVENT_REMOTE_BRIGHTNESS_UPDATED 9

namespace rvl {

class System {
protected:
  void setConnectedState(bool connected);

public:
  bool isConnected();

  virtual void loop() = 0;

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
  virtual void endRead() = 0;

  virtual uint16_t getDeviceId() = 0;

  virtual uint32_t localClock() = 0;
  virtual void print(const char* str) = 0;
  virtual void println(const char* str) = 0;
};

void init(System* system);

void loop();

} // namespace rvl

#endif // RVL_H_
