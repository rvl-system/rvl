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

#ifndef RVL_PROTOCOLS_NETWORK_STATE_H_
#define RVL_PROTOCOLS_NETWORK_STATE_H_

#include <stdint.h>

namespace NetworkState {

void init();
void loop();

void refreshNode(uint8_t node);

bool isNodeActive(uint8_t node);
uint8_t getNumNodes();
uint8_t getNextNode(uint8_t node);

bool isControllerNode(uint8_t node);
bool isControllerActive();

void refreshNodeClock(uint8_t node);
uint8_t getNextClockNode();

void refreshLocalClockSynchronization();
bool isClockSynchronizationActive();

}  // namespace NetworkState

#endif  // RVL_PROTOCOLS_NETWORK_STATE_H_
