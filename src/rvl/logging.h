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

#ifndef RVL_LOGGING_H_
#define RVL_LOGGING_H_

// These are defined such that we can do if(logLevel >= RVLogLevel.Warning) in code
enum class RVLLogLevel {
  Error = 1,
  Info = 2,
  Debug = 3
};

class RVLLoggingInterface {
 public:
  virtual RVLLogLevel getLogLevel() = 0;
  virtual void print(const char s) = 0;
  virtual void print(const char *s) = 0;
  virtual void println() = 0;
  virtual void println(const char s) = 0;
  virtual void println(const char *s) = 0;
};

#endif  // RVL_LOGGING_H_
