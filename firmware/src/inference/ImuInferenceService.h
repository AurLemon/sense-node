#pragma once

#include <Arduino.h>

#include "events/EventBus.h"

class ImuInferenceService
{
public:
  void begin(EventBus &eventBus);
  const ImuInferenceCompletedEvent &getLatestEvent() const;

private:
  static void handleModeChanged(const ModeChangedEvent &event, void *context);
  static void handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context);

  void reset();
  void appendFrame(const SensorFrame &frame);
  void runInference();
  void emitCurrentState();

  EventBus *eventBus = nullptr;
  size_t framesCollected = 0;
  size_t featureBufferLength = 0;
  float *featureBuffer = nullptr;
  float *accDeltaHistory = nullptr;
  unsigned long activeDurationMs = 0;
  ImuInferenceCompletedEvent latestEvent{};
};
