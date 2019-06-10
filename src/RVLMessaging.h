/*
Copyright (c) Bryan Hughes <bryan@nebri.us>

This file is part of Raver Lights Messaging.

Raver Lights Messaging is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Raver Lights Messaging is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Raver Lights Messaging.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RVLMESSAGING_H_
#define RVLMESSAGING_H_

#include <stdint.h>
#include <string.h>
#include <RVLLogging.h>
#include "./rvl/wave.h"

// Note: we use the old style of enums here because we regularly switch between uint8_t values and these enum values
namespace RVLPacketType {
  enum RVLPacketType {
    Palette = 1,
    Wave = 2
  };
}

class RVLTransportInterface {
 public:
  virtual void beginWrite() = 0;
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
};

enum class RVLDeviceMode { Controller, Receiver };

class RVLPlatformInterface {
 private:
  uint32_t clockOffset;
  uint8_t channel;
  RVLDeviceMode deviceMode = RVLDeviceMode::Receiver;
  RVLWaveSettings waveSettings;

 protected:
  virtual void onWaveSettingsUpdated();
  virtual void onDeviceModeUpdated();
  virtual void onClockOffsetUpdated();
  virtual void onChannelUpdated();

 public:
  virtual uint32_t getLocalTime() = 0;
  virtual uint16_t getDeviceId() = 0;
  virtual bool isNetworkAvailable() = 0;

  uint32_t getClockOffset();
  void setClockOffset(uint32_t newOffset);
  uint32_t getAnimationClock();

  uint8_t getChannel();
  void setChannel(uint8_t channel);

  RVLDeviceMode getDeviceMode();
  void setDeviceMode(RVLDeviceMode newDeviceMode);

  RVLWaveSettings* getWaveSettings();
  void setWaveSettings(RVLWaveSettings* newWaveSettings);
};

void RVLMessagingInit(
  RVLPlatformInterface* platform,
  RVLTransportInterface* transport,
  RVLLogging* logging);

void RVLMessagingLoop();

#endif  // RVLMESSAGING_H_
