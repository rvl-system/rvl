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

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "./rvl/wave.h"

// These are defined such that we can do if(logLevel >= RVLogLevel.Warning) in code
enum class RVLLogLevel {
  Error = 1,
  Info = 2,
  Debug = 3
};

class RVLLoggingInterface {
 public:
  virtual void print(const char *s) = 0;
  virtual void println() = 0;
  virtual void println(const char *s) = 0;
};

class RVLLogging {
 private:
  RVLLogLevel logLevel;
  RVLLoggingInterface* interface;

  void log(const char *s);
  void log(const char *s, va_list argptr);

 public:
  RVLLogging(RVLLoggingInterface* iface, RVLLogLevel level);

  void error(const char *s, ...);
  void info(const char *s, ...);
  void debug(const char *s, ...);
};

// Note: we use the old style of enums here because we regularly switch between uint8_t values and these enum values
namespace RVLPacketType {
  enum RVLPacketType {
    Palette = 1,
    Wave = 2
  };
}

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
};

enum class RVLDeviceMode { Controller, Receiver };

class RVLPlatformInterface {
 private:
  uint8_t channel = 0;
  RVLDeviceMode deviceMode = RVLDeviceMode::Receiver;
  RVLWaveSettings waveSettings;
  bool powerState = 0;
  uint8_t brightness = 0;
  bool synchronized = 0;

 protected:
  virtual void onWaveSettingsUpdated();
  virtual void onDeviceModeUpdated();
  virtual void onClockOffsetUpdated();
  virtual void onChannelUpdated();
  virtual void onPowerStateUpdated();
  virtual void onBrightnessUpdated();
  virtual void onSynchronizationStateUpdated();

 public:
  virtual uint32_t getLocalTime() = 0;
  virtual uint16_t getDeviceId() = 0;
  virtual bool isNetworkAvailable() = 0;

  uint32_t getAnimationClock();
  void setAnimationClock(uint32_t newClock);

  uint8_t getChannel();
  void setChannel(uint8_t channel);

  RVLDeviceMode getDeviceMode();
  void setDeviceMode(RVLDeviceMode newDeviceMode);

  RVLWaveSettings* getWaveSettings();
  void setWaveSettings(RVLWaveSettings* newWaveSettings);

  bool getPowerState();
  void setPowerState(bool newPowerState);

  uint8_t getBrightness();
  void setBrightness(uint8_t newBrightness);

  bool getSynchronizationState();
  void setSynchronizationState(bool synchronized);
};

void RVLMessagingInit(
  RVLPlatformInterface* platform,
  RVLTransportInterface* transport,
  RVLLogging* logging);

void RVLMessagingLoop();

#endif  // RVL_H_
