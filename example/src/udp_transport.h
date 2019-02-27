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

#ifndef UDP_TRANSPORT_H_
#define UDP_TRANSPORT_H_

#include <stdint.h>
#include <Arduino.h>
#include <RaverLightsMessaging.h>

#define SERVER_PORT 4978

class UDPTransport : public RVTransportInterface {
 public:
  explicit UDPTransport(WiFiUDP* udp);

  void beginWrite();
  void write8(uint8_t data);
  void write16(uint16_t data);
  void write32(uint32_t data);
  void write(uint8_t* data, uint16_t length);
  void endWrite();

  uint16_t parsePacket();
  uint8_t read8();
  uint16_t read16();
  uint32_t read32();
  void read(uint8_t* buffer, uint16_t length);
};

#endif  // UDP_TRANSPORT_H_
