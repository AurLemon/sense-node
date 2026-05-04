#pragma once

#include <Arduino.h>

#include "events/EventBus.h"

class TofHandStateService
{
public:
  void begin(EventBus &eventBus);
  const HandStateUpdatedEvent &getLatestEvent() const;

private:
  static void handleModeChanged(const ModeChangedEvent &event, void *context);
  static void handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context);

  void reset();
  void update(uint16_t distanceMm, bool tofValid);
  HandState classifyDesiredState(uint16_t distanceMm, bool tofValid) const;
  void emitCurrentState();

  EventBus *eventBus = nullptr;
  HandState currentState = HandState::NoHand;
  HandState candidateState = HandState::NoHand;
  unsigned long candidateSinceMs = 0;
  unsigned long stateSinceMs = 0;
  HandStateUpdatedEvent latestEvent{};
};
