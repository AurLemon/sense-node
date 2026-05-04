#pragma once

#include <Arduino.h>

#include "events/EventBus.h"

class FusionStateMachineService
{
public:
  void begin(EventBus &eventBus);
  const FusionDecisionEvent &getLatestEvent() const;

private:
  static void handleModeChanged(const ModeChangedEvent &event, void *context);
  static void handleImuInferenceCompleted(const ImuInferenceCompletedEvent &event, void *context);
  static void handleHandStateUpdated(const HandStateUpdatedEvent &event, void *context);
  static void handleHandLeaveDetected(const HandLeaveDetectedEvent &event, void *context);

  void reset();
  void evaluate();
  void emitCurrentState();
  void enterTapPulse(unsigned long now);
  void enterBoardMotion(unsigned long now);
  bool rawStableFor(unsigned long now, unsigned long durationMs) const;

  EventBus *eventBus = nullptr;
  ImuInferenceCompletedEvent latestImu{};
  HandStateUpdatedEvent latestHand{};
  FusionDecisionEvent latestFusion{};
  FusionState fusionState = FusionState::Idle;
  bool handLeavePending = false;
  unsigned long tapPulseUntilMs = 0;
  unsigned long tapCooldownUntilMs = 0;
  unsigned long motionStartMs = 0;
};
