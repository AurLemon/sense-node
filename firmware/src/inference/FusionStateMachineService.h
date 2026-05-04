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

  EventBus *eventBus = nullptr;
  ImuInferenceCompletedEvent latestImu{};
  HandStateUpdatedEvent latestHand{};
  FusionDecisionEvent latestFusion{};
  bool handLeavePending = false;
  unsigned long lastTapMs = 0;
  unsigned long lastBoardMotionMs = 0;
};
