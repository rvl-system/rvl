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

#ifndef RVL_SCENES_H_
#define RVL_SCENES_H_

#include "./parametric.hpp"
#include <optional>
#include <stdint.h>
#include <variant>

struct RVLOff {};

inline bool operator==(const RVLOff& /*a*/, const RVLOff& /*b*/) {
  return true;
}

using RVLSceneContent = std::variant<RVLOff, RVLParametricSettings>;

// What a channel shows from its start frame on, dissolving over its fade from
// whatever was showing. Never changed once scheduled
struct RVLScene {
  uint32_t start; // frame number
  uint8_t fade; // frames
  RVLSceneContent content;
};

inline bool operator==(const RVLScene& a, const RVLScene& b) {
  return a.start == b.start && a.fade == b.fade && a.content == b.content;
}

namespace rvl {

enum class AnimationType : uint8_t { Off, Parametric };

// Previous dissolving into current, amount out of 255, while fading
struct RenderPlan {
  RVLScene current;
  RVLScene previous;
  uint32_t frame;
  uint8_t amount;
  bool fading;
};

// A scene from a controller. One this node already holds is a re-send
void scheduleScene(const RVLScene& scene);
RVLScene getCurrentScene();
std::optional<RVLScene> getPendingScene();
RenderPlan getRenderPlan();

// A controller board's own changes. Each becomes a scene a lead from now once
// the last one has finished fading, and until then the latest is held
void setParametricSettings(RVLParametricSettings* newSettings);
void setOff();

// The current scene. Read the settings under the lock; off reads as black
AnimationType getAnimationType();
RVLParametricSettings* getParametricSettings();

namespace Scenes {

void init();
void loop();

// For adjustAnimationClock, under the lock, on a step of a frame or more.
// Returns whether it scheduled a scene, which the caller announces once
// unlocked
bool onClockStep(int32_t shift, uint32_t currentFrame);

// Back to the boot scene, for tests
void reset();

} // namespace Scenes

} // namespace rvl

#endif // RVL_SCENES_H_
