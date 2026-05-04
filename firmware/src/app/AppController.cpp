#include "app/AppController.h"

namespace
{
  constexpr uint32_t kSerialBaudRate = 115200;
  constexpr unsigned long kSampleIntervalMs = 50;
}

void AppController::setup()
{
  Serial.begin(kSerialBaudRate);
  delay(1000);

  hardware.beginModeSwitch();
  currentMode = hardware.readModeSwitch();
  const bool verboseBoot = currentMode == AppMode::Demo;

  hardware.beginBase();
  hardware.beginSensors(verboseBoot);

  inferenceService.begin(eventBus);
  demoMode.begin(eventBus, hardware);
  captureMode.begin(eventBus, hardware);
  eventBus.emitModeChanged({currentMode, currentMode});

  previousFrame = latestFrame;
  latestFrame = hardware.readSensors();
  eventBus.emitSensorFrameSampled({latestFrame, previousFrame, currentMode});
  eventBus.emitDemoSerialTick({latestFrame, InferenceCompletedEvent{}});

  lastSampleMs = millis();
  lastDemoSerialMs = millis();
}

void AppController::loop()
{
  const AppMode nextMode = hardware.readModeSwitch();
  if (nextMode != currentMode)
  {
    changeMode(nextMode);
  }

  const unsigned long now = millis();
  if (now - lastSampleMs >= kSampleIntervalMs)
  {
    lastSampleMs = now;
    previousFrame = latestFrame;
    latestFrame = hardware.readSensors();
    eventBus.emitSensorFrameSampled({latestFrame, previousFrame, currentMode});
    if (currentMode == AppMode::Demo)
    {
      lastDemoSerialMs = now;
      eventBus.emitDemoSerialTick({latestFrame, inferenceService.getLatestEvent()});
    }
  }

  hardware.updateUserLed(currentMode);
  delay(1);
}

void AppController::changeMode(AppMode nextMode)
{
  const AppMode previousMode = currentMode;
  currentMode = nextMode;
  eventBus.emitModeChanged({previousMode, currentMode});
}
