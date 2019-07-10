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

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "RVLLogging.h"

RVLLogging::RVLLogging(RVLLoggingInterface* iface, RVLLogLevel level) {
  this->interface = iface;
  this->logLevel = level;
}

void RVLLogging::log(const char *s) {
  this->interface->print(s);
}

void RVLLogging::log(const char *s, va_list argptr) {
  int bufferLength = strlen(s) * 3;
  char str[bufferLength];
  vsnprintf(str, bufferLength, s, argptr);
  this->interface->print(str);
}

void RVLLogging::error(const char *s, ...) {
  if (this->logLevel >= RVLLogLevel::Error) {
    this->interface->print("[error]: ");
    va_list argptr;
    va_start(argptr, s);
    this->log(s, argptr);
    this->interface->println();
  }
}

void RVLLogging::info(const char *s, ...) {
  if (this->logLevel >= RVLLogLevel::Info) {
    this->interface->print("[info ]: ");
    va_list argptr;
    va_start(argptr, s);
    this->log(s, argptr);
    this->interface->println();
  }
}

void RVLLogging::debug(const char *s, ...) {
  if (this->logLevel >= RVLLogLevel::Debug) {
    this->interface->print("[debug ]: ");
    va_list argptr;
    va_start(argptr, s);
    this->log(s, argptr);
    this->interface->println();
  }
}
