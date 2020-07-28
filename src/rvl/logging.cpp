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

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "./rvl.h"
#include "./rvl/logging.h"
#include "./rvl/platform.h"

namespace rvl {

LogLevel logLevel = LogLevel::Debug;

void setLogLevel(LogLevel newLevel) {
  logLevel = newLevel;
}

void log(const char *s) {
  Platform::system->print(s);
}

void log(const char *s, va_list argptr) {
  int bufferLength = strlen(s) * 3;
  char* str = new char[bufferLength];
  vsnprintf(str, bufferLength, s, argptr);
  Platform::system->print(str);
  delete[] str;
}

void error(const char *s, ...) {
  if (logLevel >= LogLevel::Error) {
    Platform::system->print("[error]: ");
    va_list argptr;
    va_start(argptr, s);
    log(s, argptr);
    Platform::system->println("");
  }
}

void info(const char *s, ...) {
  if (logLevel >= LogLevel::Info) {
    Platform::system->print("[info ]: ");
    va_list argptr;
    va_start(argptr, s);
    log(s, argptr);
    Platform::system->println("");
  }
}

void debug(const char *s, ...) {
  if (logLevel >= LogLevel::Debug) {
    Platform::system->print("[debug]: ");
    va_list argptr;
    va_start(argptr, s);
    log(s, argptr);
    Platform::system->println("");
  }
}

}  // namespace rvl
