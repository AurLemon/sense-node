#include "modes/Modes.h"

void DemoMode::begin(EventBus &eventBus, Hardware &targetHardware)
{
  hardware = &targetHardware;
  eventBus.onModeChanged(handleModeChanged, this);
  eventBus.onSensorFrameSampled(handleSensorFrameSampled, this);
  eventBus.onDemoSerialTick(handleDemoSerialTick, this);
  eventBus.onFusionDecision(handleFusionDecision, this);
}

void DemoMode::handleModeChanged(const ModeChangedEvent &event, void *context)
{
  auto *self = static_cast<DemoMode *>(context);
  if (event.currentMode != AppMode::Demo)
  {
    return;
  }

  if (event.previousMode != event.currentMode)
  {
    Serial.println();
    Serial.println("Demo mode");
  }
  self->currentFrame = SensorFrame{};
  strncpy(self->displayLabel, "warming_up", sizeof(self->displayLabel) - 1);
  self->displayLabel[sizeof(self->displayLabel) - 1] = '\0';
  self->renderCurrentFrame();
}

void DemoMode::handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context)
{
  auto *self = static_cast<DemoMode *>(context);
  if (event.currentMode != AppMode::Demo)
  {
    return;
  }

  self->currentFrame = event.current;
  self->renderCurrentFrame();
}

void DemoMode::handleDemoSerialTick(const DemoSerialTickEvent &event, void *context)
{
  auto *self = static_cast<DemoMode *>(context);
  self->printDemoCsvLine(event);
}

void DemoMode::handleFusionDecision(const FusionDecisionEvent &event, void *context)
{
  auto *self = static_cast<DemoMode *>(context);
  const char *nextLabel = event.ready ? toString(event.finalEvent) : "warming_up";
  strncpy(self->displayLabel, nextLabel, sizeof(self->displayLabel) - 1);
  self->displayLabel[sizeof(self->displayLabel) - 1] = '\0';
  self->renderCurrentFrame();
}

void DemoMode::printFrameToSerial(const SensorFrame &frame)
{
  Serial.println();

  if (frame.tofValid)
  {
    Serial.print("TOF: ");
    Serial.print(static_cast<float>(frame.tofMm), 5);
    Serial.println(" mm");
  }
  else
  {
    Serial.println("TOF: --");
  }

  if (frame.imuValid)
  {
    Serial.print("A: ");
    Serial.print(frame.accel.acceleration.x, 5);
    Serial.print(", ");
    Serial.print(frame.accel.acceleration.y, 5);
    Serial.print(", ");
    Serial.println(frame.accel.acceleration.z, 5);

    Serial.print("G: ");
    Serial.print(frame.gyro.gyro.x, 5);
    Serial.print(", ");
    Serial.print(frame.gyro.gyro.y, 5);
    Serial.print(", ");
    Serial.println(frame.gyro.gyro.z, 5);
  }
  else
  {
    Serial.println("IMU: --");
  }
}

void DemoMode::printDemoCsvLine(const DemoSerialTickEvent &event)
{
  const SensorFrame &frame = event.current;
  const unsigned long uptimeMs = millis();
  const unsigned long uptimeWholeSeconds = uptimeMs / 1000;
  const unsigned long uptimeTenths = (uptimeMs % 1000) / 100;

  if (frame.tofValid)
  {
    Serial.print(frame.tofMm);
  }
  else
  {
    Serial.print(-1);
  }

  Serial.print(',');

  if (frame.imuValid)
  {
    Serial.print(frame.accel.acceleration.x, 4);
    Serial.print(',');
    Serial.print(frame.accel.acceleration.y, 4);
    Serial.print(',');
    Serial.print(frame.accel.acceleration.z, 4);
    Serial.print(',');
    Serial.print(frame.gyro.gyro.x, 4);
    Serial.print(',');
    Serial.print(frame.gyro.gyro.y, 4);
    Serial.print(',');
    Serial.print(frame.gyro.gyro.z, 4);
  }
  else
  {
    Serial.print("-1.0000,-1.0000,-1.0000,-1.0000,-1.0000,-1.0000");
  }

  Serial.print(',');
  Serial.print(uptimeWholeSeconds);
  Serial.print('.');
  Serial.print(uptimeTenths);
  Serial.print('s');
  Serial.print(',');
  Serial.print(event.imu.ready && event.imu.windowReady ? event.imu.rawLabel : "warming_up");
  Serial.print(',');
  Serial.print(event.imu.ready ? event.imu.confidence : 0.0f, 4);
  Serial.print(',');
  Serial.print(toString(event.hand.handState));
  Serial.print(',');
  Serial.println(event.fusion.ready ? toString(event.fusion.finalEvent) : "warming_up");
}

void DemoMode::renderCurrentFrame()
{
  hardware->renderDemo(currentFrame, displayLabel);
}

void CaptureMode::begin(EventBus &eventBus, Hardware &targetHardware)
{
  hardware = &targetHardware;
  eventBus.onModeChanged(handleModeChanged, this);
  eventBus.onSensorFrameSampled(handleSensorFrameSampled, this);
}

void CaptureMode::handleModeChanged(const ModeChangedEvent &event, void *context)
{
  auto *self = static_cast<CaptureMode *>(context);
  if (event.currentMode == AppMode::Capture)
  {
    self->hardware->renderCaptureMode();
  }
}

void CaptureMode::handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context)
{
  auto *self = static_cast<CaptureMode *>(context);
  if (event.currentMode != AppMode::Capture)
  {
    return;
  }

  self->printCsvFrame(event.current);
}

void CaptureMode::printCsvFrame(const SensorFrame &frame)
{
  if (frame.tofValid)
  {
    Serial.print(frame.tofMm);
  }
  else
  {
    Serial.print(-1);
  }

  Serial.print(',');

  if (frame.imuValid)
  {
    Serial.print(frame.accel.acceleration.x, 4);
    Serial.print(',');
    Serial.print(frame.accel.acceleration.y, 4);
    Serial.print(',');
    Serial.print(frame.accel.acceleration.z, 4);
    Serial.print(',');
    Serial.print(frame.gyro.gyro.x, 4);
    Serial.print(',');
    Serial.print(frame.gyro.gyro.y, 4);
    Serial.print(',');
    Serial.println(frame.gyro.gyro.z, 4);
    return;
  }

  Serial.println("-1.0000,-1.0000,-1.0000,-1.0000,-1.0000,-1.0000");
}
