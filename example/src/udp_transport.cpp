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

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "./udp_transport.h"

WiFiUDP* udp;
IPAddress SERVER_IP(192, 168, 1, 1);
IPAddress GATEWAY(192, 168, 1, 255);
IPAddress SUBNET(255, 255, 255, 0);

UDPTransport::UDPTransport(WiFiUDP* newUDP) {
  udp = newUDP;
}

void UDPTransport::beginWrite() {
  udp->beginPacket(GATEWAY, SERVER_PORT);
}

void UDPTransport::write8(uint8_t data) {
  udp->write(data);
}

void UDPTransport::write16(uint16_t data) {
  udp->write(data >> 8);
  udp->write(data & 0xFF);
}

void UDPTransport::write32(uint32_t data) {
  udp->write(data >> 24);
  udp->write(data >> 16 & 0xFF);
  udp->write(data >> 8 & 0xFF);
  udp->write(data & 0xFF);
}

void UDPTransport::write(uint8_t* data, uint16_t length) {
  udp->write(data, length);
}

void UDPTransport::endWrite() {
  udp->endPacket();
}

uint16_t UDPTransport::parsePacket() {
  return udp->parsePacket();
}

uint8_t UDPTransport::read8() {
  return udp->read();
}

uint16_t UDPTransport::read16() {
  uint16_t val = 0;
  val |= udp->read() << 8;
  val |= udp->read();
  return val;
}

uint32_t UDPTransport::read32() {
  uint32_t val = 0;
  val |= udp->read() << 24;
  val |= udp->read() << 16;
  val |= udp->read() << 8;
  val |= udp->read();
  return val;
}

void UDPTransport::read(uint8_t* buffer, uint16_t length) {
  udp->read(buffer, length);
}
