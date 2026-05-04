#pragma once

#include <Arduino.h>

#include "hardware/Hardware.h"

enum class AppMode
{
  Demo,
  Capture
};

struct ModeChangedEvent
{
  AppMode previousMode;
  AppMode currentMode;
};

struct SensorFrameSampledEvent
{
  SensorFrame current;
  SensorFrame previous;
  AppMode currentMode;
};

struct InferenceCompletedEvent
{
  bool ready = false;
  bool sensorWindowReady = false;
  float confidence = 0.0f;
  char rawLabel[24]{};
  char displayLabel[24]{};
};

struct DemoSerialTickEvent
{
  SensorFrame current;
  InferenceCompletedEvent inference;
};

class EventBus
{
public:
  using ModeChangedHandler = void (*)(const ModeChangedEvent &, void *);
  using SensorFrameSampledHandler = void (*)(const SensorFrameSampledEvent &, void *);
  using DemoSerialTickHandler = void (*)(const DemoSerialTickEvent &, void *);
  using InferenceCompletedHandler = void (*)(const InferenceCompletedEvent &, void *);

  void onModeChanged(ModeChangedHandler handler, void *context);
  void onSensorFrameSampled(SensorFrameSampledHandler handler, void *context);
  void onDemoSerialTick(DemoSerialTickHandler handler, void *context);
  void onInferenceCompleted(InferenceCompletedHandler handler, void *context);

  void emitModeChanged(const ModeChangedEvent &event);
  void emitSensorFrameSampled(const SensorFrameSampledEvent &event);
  void emitDemoSerialTick(const DemoSerialTickEvent &event);
  void emitInferenceCompleted(const InferenceCompletedEvent &event);

private:
  static constexpr uint8_t kMaxHandlers = 4;

  struct ModeChangedSubscription
  {
    ModeChangedHandler handler = nullptr;
    void *context = nullptr;
  };

  struct SensorFrameSampledSubscription
  {
    SensorFrameSampledHandler handler = nullptr;
    void *context = nullptr;
  };

  struct DemoSerialTickSubscription
  {
    DemoSerialTickHandler handler = nullptr;
    void *context = nullptr;
  };

  struct InferenceCompletedSubscription
  {
    InferenceCompletedHandler handler = nullptr;
    void *context = nullptr;
  };

  ModeChangedSubscription modeChangedHandlers[kMaxHandlers]{};
  SensorFrameSampledSubscription sensorFrameHandlers[kMaxHandlers]{};
  DemoSerialTickSubscription demoSerialHandlers[kMaxHandlers]{};
  InferenceCompletedSubscription inferenceHandlers[kMaxHandlers]{};
};
