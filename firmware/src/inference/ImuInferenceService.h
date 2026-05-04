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
  void updateGravityBaseline(float accMag, bool quietDetected);

public:
  static constexpr size_t kRecentMetricsWindow = 8;

  struct RecentMotionSample
  {
    unsigned long timestampMs = 0;
    float accDelta = 0.0f;
    float accJerk = 0.0f;
    float gyroMag = 0.0f;
    bool active = false;
    bool stable = false;
  };

private:
  EventBus *eventBus = nullptr;
  size_t framesCollected = 0;
  size_t featureBufferLength = 0;
  float *featureBuffer = nullptr;
  float *accDeltaHistory = nullptr;
  unsigned long activeDurationMs = 0;
  unsigned long motionActiveSinceMs = 0;
  unsigned long stableSinceMs = 0;
  float lastAccMag = 9.8f;
  float gravityBaseline = 9.8f;
  bool gravityBaselineInitialized = false;
  RecentMotionSample recentMetrics[kRecentMetricsWindow]{};
  size_t recentMetricsCount = 0;
  size_t recentMetricsHead = 0;
  ImuInferenceCompletedEvent latestEvent{};
};
