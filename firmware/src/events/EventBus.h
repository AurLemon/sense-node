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

struct DemoSerialTickEvent
{
  SensorFrame current;
};

class EventBus
{
public:
  using ModeChangedHandler = void (*)(const ModeChangedEvent &, void *);
  using SensorFrameSampledHandler = void (*)(const SensorFrameSampledEvent &, void *);
  using DemoSerialTickHandler = void (*)(const DemoSerialTickEvent &, void *);

  void onModeChanged(ModeChangedHandler handler, void *context);
  void onSensorFrameSampled(SensorFrameSampledHandler handler, void *context);
  void onDemoSerialTick(DemoSerialTickHandler handler, void *context);

  void emitModeChanged(const ModeChangedEvent &event);
  void emitSensorFrameSampled(const SensorFrameSampledEvent &event);
  void emitDemoSerialTick(const DemoSerialTickEvent &event);

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

  ModeChangedSubscription modeChangedHandlers[kMaxHandlers]{};
  SensorFrameSampledSubscription sensorFrameHandlers[kMaxHandlers]{};
  DemoSerialTickSubscription demoSerialHandlers[kMaxHandlers]{};
};
