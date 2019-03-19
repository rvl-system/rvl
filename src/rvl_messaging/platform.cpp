/*
Copyright (c) 2018 Bryan Hughes <bryan@nebri.us>

This file is part of Raver Lights.

Raver Lights is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Raver Lights is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Raver Lights.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "./RVLMessaging.h"
#include "./rvl_messaging/platform.h"

namespace Platform {

RVLTransportInterface* transport;
RVLLogging* logging;
RVLPlatformInterface* platform;

// Base case versions

void init(RVLPlatformInterface* newPlatform, RVLTransportInterface* newTransport, RVLLogging* newLogging) {
  platform = newPlatform;
  transport = newTransport;
  logging = newLogging;
}

}  // namespace Platform
