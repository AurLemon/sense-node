#include "app/AppController.h"

namespace
{
  constexpr uint32_t kSerialBaudRate = 115200;
  constexpr unsigned long kSampleIntervalMs = 50;
  constexpr unsigned long kCaptureRenderIntervalMs = 250;
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

  imuInferenceService.begin(eventBus);
  tofHandStateService.begin(eventBus);
  fusionStateMachineService.begin(eventBus);
  demoMode.begin(eventBus, hardware);
  captureMode.begin(eventBus, hardware);
  eventBus.emitModeChanged({currentMode, currentMode});

  previousFrame = latestFrame;
  latestFrame = hardware.readSensors();
  eventBus.emitSensorFrameSampled({latestFrame, previousFrame, currentMode});
  eventBus.emitDemoSerialTick(
      {latestFrame, imuInferenceService.getLatestEvent(), tofHandStateService.getLatestEvent(),
       fusionStateMachineService.getLatestEvent()});

  lastSampleMs = millis();
  lastDemoSerialMs = millis();
  lastCaptureRenderMs = millis();
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
      eventBus.emitDemoSerialTick(
          {latestFrame, imuInferenceService.getLatestEvent(), tofHandStateService.getLatestEvent(),
           fusionStateMachineService.getLatestEvent()});
    }
  }

  if (currentMode == AppMode::Capture && now - lastCaptureRenderMs >= kCaptureRenderIntervalMs)
  {
    lastCaptureRenderMs = now;
    hardware.renderCaptureMode();
  }

  hardware.updateUserLed(currentMode);
  delay(1);
}

void AppController::changeMode(AppMode nextMode)
{
  const AppMode previousMode = currentMode;
  currentMode = nextMode;
  if (currentMode == AppMode::Capture)
  {
    lastCaptureRenderMs = millis();
  }
  eventBus.emitModeChanged({previousMode, currentMode});
}
