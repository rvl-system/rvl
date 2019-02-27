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
#include <RaverLightsMessaging.h>
#include "./arduino_platform.h"
#include "./udp_transport.h"
#include "./serial_logging.h"

#define STATE_DISCONNECTED 0
#define STATE_CONNECTING 1
#define STATE_CONNECTED 2

byte state = STATE_DISCONNECTED;

WiFiUDP udp;
UDPTransport transport(&udp);
SerialLogging logging(RVLogLevel::Info);
ArduinoPlatform platform;

void setup() {
  WiFi.begin("network-name", "pass-to-network");

  logging.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    logging.print(".");
  }
  logging.println("");

  logging.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  udp.begin(SERVER_PORT);

  platform.setDeviceId(WiFi.localIP()[3]);

  initRaverLightsMessaging(&platform, &transport, &logging);
}

void loop() {
  loopRaverLightsMessaging();
  delay(10);
}
