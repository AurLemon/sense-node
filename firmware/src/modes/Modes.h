#pragma once

#include <Arduino.h>

#include "events/EventBus.h"
#include "hardware/Hardware.h"
#include "inference/InteractionTypes.h"

class DemoMode
{
public:
  void begin(EventBus &eventBus, Hardware &hardware);

private:
  static void handleModeChanged(const ModeChangedEvent &event, void *context);
  static void handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context);
  static void handleDemoSerialTick(const DemoSerialTickEvent &event, void *context);
  static void handleFusionDecision(const FusionDecisionEvent &event, void *context);
  void printFrameToSerial(const SensorFrame &frame);
  void printDemoCsvLine(const DemoSerialTickEvent &event);
  void renderCurrentFrame();

  Hardware *hardware = nullptr;
  SensorFrame currentFrame{};
  char displayLabel[24] = "warming_up";
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
