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

#ifndef RVL_LOGGING_H_
#define RVL_LOGGING_H_

#include <stdint.h>

namespace rvl {

// These are defined such that we can do if(logLevel >= RVLogLevel.Warning) in code
enum class LogLevel {
  Error = 1,
  Info = 2,
  Debug = 3
};

void setLogLevel(LogLevel level);

void error(const char *s, ...);
void info(const char *s, ...);
void debug(const char *s, ...);

void on(uint8_t eventType, void (*listener)());
void emit(uint8_t eventType);

}  // namespace rvl

#endif  // RVL_LOGGING_H_
