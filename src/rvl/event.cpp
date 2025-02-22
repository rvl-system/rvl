/*
Copyright (c) Bryan Hughes <bryan@nebri.us>

This file is part of RVL.

RVL is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RVL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RVL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "./rvl/event.hpp"
#include "./rvl.hpp"
#include <list>
#include <stdint.h>

namespace rvl {

struct ListenerEntry {
public:
  int eventType;
  void (*listener)();
};
std::list<ListenerEntry> listeners;

void on(uint8_t eventType, void (*listener)()) {
  ListenerEntry entry = {eventType, listener};
  listeners.push_back(entry);
}

void emit(uint8_t eventType) {
  for (auto& listener : listeners) {
    if (listener.eventType == eventType) {
      listener.listener();
    }
  }
}

} // namespace rvl
