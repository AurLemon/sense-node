#pragma once

#include <Arduino.h>

#include "events/EventBus.h"
#include "hardware/Hardware.h"
#include "inference/InferenceService.h"
#include "modes/Modes.h"

class AppController
{
public:
  void setup();
  void loop();

private:
  void changeMode(AppMode nextMode);

  EventBus eventBus;
  Hardware hardware;
  InferenceService inferenceService;
  DemoMode demoMode;
  CaptureMode captureMode;
  AppMode currentMode = AppMode::Demo;
  SensorFrame latestFrame{};
  SensorFrame previousFrame{};
  unsigned long lastSampleMs = 0;
  unsigned long lastDemoSerialMs = 0;
};
