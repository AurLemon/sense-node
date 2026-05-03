#pragma once

#include <Arduino.h>

#include "events/EventBus.h"
#include "hardware/Hardware.h"

class DemoMode
{
public:
  void begin(EventBus &eventBus, Hardware &hardware);

private:
  static void handleModeChanged(const ModeChangedEvent &event, void *context);
  static void handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context);
  static void handleDemoSerialTick(const DemoSerialTickEvent &event, void *context);
  void printFrameToSerial(const SensorFrame &frame);

  Hardware *hardware = nullptr;
};

class CaptureMode
{
public:
  void begin(EventBus &eventBus, Hardware &hardware);

private:
  static void handleModeChanged(const ModeChangedEvent &event, void *context);
  static void handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context);
  void printCsvFrame(const SensorFrame &frame);

  Hardware *hardware = nullptr;
};
