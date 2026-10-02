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

#include "./rvl/scenes.hpp"
#include "./rvl.hpp"
#include "./rvl/config.hpp"
#include "./rvl/platform.hpp"
#include <algorithm>
#include <optional>

namespace rvl {

#define LATE_REPORT_INTERVAL 60000

namespace {

// Off, so a node's first scene fades in from black. No scene off the wire can
// equal it, since every one has a fade
static_assert(MIN_FADE_FRAMES > 0);
const RVLScene BOOT_SCENE = {0, 0, RVLOff{}};

// Previous dissolves into current over [fadeStart, fadeEnd). Pending waits for
// its start. Requested is a controller's change the gate is holding
RVLScene previous = BOOT_SCENE;
RVLScene current = BOOT_SCENE;
std::optional<RVLScene> pending;
std::optional<RVLSceneContent> requested;
uint32_t fadeStart = 0;
uint32_t fadeEnd = 0;

uint32_t lateActivations = 0;
int32_t largestLateness = 0;
uint32_t nextLateReportTime = 0;

RVLParametricSettings black;

bool isFading(uint32_t currentFrame) {
  return subtractFrames(currentFrame, fadeStart) >= 0 &&
      subtractFrames(currentFrame, fadeEnd) < 0;
}

bool isStartDue(const RVLScene& scene, uint32_t currentFrame) {
  return subtractFrames(currentFrame, scene.start) >= 0;
}

// Fades from now toward the end the sender intended, never shorter than the
// floor. Content that isn't changing has nothing to dissolve
void activateScene(const RVLScene& scene, uint32_t currentFrame) {
  // Later than a whole fade is catching up after a reboot or a channel switch,
  // not a missed lead
  int32_t lateness = subtractFrames(currentFrame, scene.start);
  if (lateness > 0 && lateness <= UINT8_MAX) {
    lateActivations++;
    largestLateness = std::max(largestLateness, lateness);
  }

  uint32_t end = currentFrame;
  if (!(scene.content == current.content)) {
    int32_t remaining = subtractFrames(scene.start + scene.fade, currentFrame);
    end = currentFrame +
        std::clamp<int32_t>(remaining, MIN_FADE_FRAMES, UINT8_MAX);
  }

  // The render reads these four together on the other core
  lockState();
  previous = current;
  current = scene;
  fadeStart = currentFrame;
  fadeEnd = end;
  freeState();
}

// Returns whether the scene was accepted
bool activateOrPendScene(const RVLScene& scene, uint32_t currentFrame) {
  // Re-sends. Previous is here for the sender's in-flight copy of the scene
  // this node has just left
  if (scene == previous || scene == current || (pending && scene == *pending)) {
    return false;
  }
  bool due = isStartDue(scene, currentFrame);
  if (due && !isFading(currentFrame)) {
    // A due scene still waiting on the fade that just ended is superseded
    if (pending && isStartDue(*pending, currentFrame)) {
      pending.reset();
    }
    activateScene(scene, currentFrame);
    return true;
  }
  // What the fleet shows now outranks what it will show next, which comes back
  // with the next re-send
  if (!due && pending && isStartDue(*pending, currentFrame)) {
    return false;
  }
  pending = scene;
  return true;
}

// A controller's own scene always starts in the future, so it pends and never
// reaches activateScene's lock. The clock-step hook calls this with the lock
// already held
static_assert(SCENE_LEAD_FRAMES > 0);

// Starts the lead after currentFrame, or when a running fade ends if that's
// later. The fade is floored, as the parser floors one off the wire: the hook
// and the re-key copy current's, which is 0 on the boot scene, and the sender
// relies on no other scene having none. Returns whether the scene was accepted
bool scheduleNextScene(
    uint32_t currentFrame, uint8_t fade, const RVLSceneContent& content) {
  uint32_t start = currentFrame + SCENE_LEAD_FRAMES;
  if (isFading(currentFrame) && subtractFrames(fadeEnd, start) > 0) {
    start = fadeEnd;
  }
  return activateOrPendScene(
      {start, std::max<uint8_t>(fade, MIN_FADE_FRAMES), content}, currentFrame);
}

// The gate. A receiver never originates a scene, and content already scheduled
// is dropped. Returns whether it scheduled a scene
bool scheduleOrHoldContent(const RVLSceneContent& content) {
  if (getDeviceMode() != DeviceMode::Controller) {
    return false;
  }
  uint32_t currentFrame = getAnimationFrame();
  const RVLSceneContent& target = pending ? pending->content : current.content;
  if (content == target) {
    requested.reset();
    return false;
  }
  if (pending || isFading(currentFrame)) {
    requested = content;
    return false;
  }
  requested.reset();
  return scheduleNextScene(currentFrame, DEFAULT_FADE_FRAMES, content);
}

void resetToBootScene() {
  pending.reset();
  requested.reset();
  // The render reads these four together on the other core
  lockState();
  previous = BOOT_SCENE;
  current = BOOT_SCENE;
  fadeStart = 0;
  fadeEnd = 0;
  freeState();
}

void announceIfScheduled(bool scheduled) {
  if (scheduled) {
    emit(EVENT_ANIMATION_UPDATED);
  }
}

void onChannelUpdated() {
  if (getDeviceMode() == DeviceMode::Controller) {
    emit(EVENT_ANIMATION_UPDATED);
    return;
  }
  resetToBootScene();
}

// A new controller doesn't reset. The fleet's scene stays current until the
// controller's first scene, sent now and starting a lead later, activates on
// every board, so they all dissolve from the fleet's scene into the preset
void onDeviceModeUpdated() {
  if (getDeviceMode() == DeviceMode::Receiver) {
    resetToBootScene();
  } else {
    pending.reset();
    requested.reset();
  }
}

void reportLateActivations() {
  uint32_t now = Platform::system->localClock();
  if (static_cast<int32_t>(now - nextLateReportTime) < 0) {
    return;
  }
  nextLateReportTime = now + LATE_REPORT_INTERVAL;
  if (lateActivations > 0) {
    info("Late scene activations in the last minute: %u, by up to %d frames",
        static_cast<unsigned int>(lateActivations),
        static_cast<int>(largestLateness));
  }
  lateActivations = 0;
  largestLateness = 0;
}

} // namespace

void scheduleScene(const RVLScene& scene) {
  announceIfScheduled(activateOrPendScene(scene, getAnimationFrame()));
}

RVLScene getCurrentScene() {
  return current;
}

std::optional<RVLScene> getPendingScene() {
  return pending;
}

// Called from the render on the other core, while the background task writes
// the scenes, the fade window and the clock offset. The frame is read under the
// lock too, so it and the window are one snapshot. While fading, the elapsed
// frames are within the window, which is at least the floor long
RenderPlan getRenderPlan() {
  RenderPlan plan;
  lockState();
  plan.frame = getAnimationFrame();
  plan.current = current;
  plan.previous = previous;
  plan.fading = isFading(plan.frame);
  plan.amount = 0;
  if (plan.fading) {
    plan.amount = subtractFrames(plan.frame, fadeStart) * UINT8_MAX /
        subtractFrames(fadeEnd, fadeStart);
  }
  freeState();
  return plan;
}

void setParametricSettings(RVLParametricSettings* newSettings) {
  announceIfScheduled(scheduleOrHoldContent(*newSettings));
}

void setOff() {
  announceIfScheduled(scheduleOrHoldContent(RVLOff{}));
}

AnimationType getAnimationType() {
  // The render calls this on the other core
  lockState();
  bool off = std::holds_alternative<RVLOff>(current.content);
  freeState();
  return off ? AnimationType::Off : AnimationType::Parametric;
}

RVLParametricSettings* getParametricSettings() {
  auto* settings = std::get_if<RVLParametricSettings>(&current.content);
  return settings != nullptr ? settings : &black;
}

namespace Scenes {

void init() {
  nextLateReportTime = Platform::system->localClock() + LATE_REPORT_INTERVAL;
  on(EVENT_CHANNEL_UPDATED, onChannelUpdated);
  on(EVENT_DEVICE_MODE_UPDATED, onDeviceModeUpdated);
}

// Every condition is a level, not an edge, so an iteration held up past the
// frame a fade ends still acts on it
void loop() {
  uint32_t currentFrame = getAnimationFrame();
  // An ended window is emptied, or it would read as fading again one full frame
  // cycle later
  if (fadeStart != fadeEnd && subtractFrames(currentFrame, fadeEnd) >= 0) {
    // The render reads the window on the other core
    lockState();
    fadeStart = fadeEnd;
    freeState();
  }
  if (pending && isStartDue(*pending, currentFrame) && !isFading(currentFrame))
  {
    RVLScene scene = *pending;
    pending.reset();
    activateScene(scene, currentFrame);
  }
  bool scheduled = false;
  if (!pending && !isFading(currentFrame)) {
    if (requested) {
      scheduled =
          scheduleNextScene(currentFrame, DEFAULT_FADE_FRAMES, *requested);
      requested.reset();
    } else if (getDeviceMode() == DeviceMode::Controller &&
        subtractFrames(currentFrame, current.start) > SCENE_MAX_AGE_FRAMES)
    {
      // Only the sender knows a scene's age, and past 2^26 frames a receiver
      // would read its start as the future
      scheduled =
          scheduleNextScene(currentFrame, current.fade, current.content);
    }
  }
  announceIfScheduled(scheduled);
  reportLateActivations();
}

// The fade is re-based rather than ended, so its amount doesn't jump. Pending
// and the held request are in the old clock's frames and go
bool onClockStep(int32_t shift, uint32_t currentFrame) {
  fadeStart += shift;
  fadeEnd += shift;
  RVLScene newest = pending.value_or(current);
  RVLSceneContent content = requested.value_or(newest.content);
  uint8_t fade = requested ? DEFAULT_FADE_FRAMES : newest.fade;
  pending.reset();
  requested.reset();
  if (getDeviceMode() != DeviceMode::Controller) {
    return false;
  }
  return scheduleNextScene(currentFrame, fade, content);
}

void reset() {
  resetToBootScene();
  lateActivations = 0;
  largestLateness = 0;
  nextLateReportTime = Platform::system->localClock() + LATE_REPORT_INTERVAL;
}

} // namespace Scenes

} // namespace rvl
