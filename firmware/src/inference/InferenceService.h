#pragma once

#include <Arduino.h>

#include "events/EventBus.h"

class InferenceService
{
public:
  void begin(EventBus &eventBus);
  const InferenceCompletedEvent &getLatestEvent() const;

private:
  static void handleModeChanged(const ModeChangedEvent &event, void *context);
  static void handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context);

  void reset();
  void appendFrame(const SensorFrame &frame);
  void emitCurrentState();
  void runInference();

  EventBus *eventBus = nullptr;
  size_t framesCollected = 0;
  size_t featureBufferLength = 0;
  float *featureBuffer = nullptr;
  InferenceCompletedEvent latestEvent{};
};
