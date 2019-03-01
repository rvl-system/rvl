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

#ifndef RVL_MESSAGING_PLATFORM_H_
#define RVL_MESSAGING_PLATFORM_H_

#include "./RaverLightsMessaging.h"

namespace Platform {

extern RVLTransportInterface* transport;
extern RVLLoggingInterface* logging;
extern RVLPlatformInterface* platform;

// Base case versions

void init(RVLPlatformInterface* newPlatform, RVLTransportInterface* newTransport, RVLLoggingInterface* newLogging);

void error(const char *s);
void info(const char *s);
void debug(const char *s);

// Variadic versions

template<typename T, typename... Args>
void error(const char *s, T value, Args... args);

template<typename T, typename... Args>
void info(const char *s, T value, Args... args);

template<typename T, typename... Args>
void debug(const char *s, T value, Args... args);

}  // namespace Platform

#endif  // RVL_MESSAGING_PLATFORM_H_
