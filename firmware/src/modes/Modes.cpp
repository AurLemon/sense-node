#include "modes/Modes.h"

void DemoMode::begin(EventBus &eventBus, Hardware &targetHardware)
{
  hardware = &targetHardware;
  eventBus.onModeChanged(handleModeChanged, this);
  eventBus.onSensorFrameSampled(handleSensorFrameSampled, this);
  eventBus.onDemoSerialTick(handleDemoSerialTick, this);
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
  self->hardware->renderDemo(SensorFrame{}, SensorFrame{});
}

void DemoMode::handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context)
{
  auto *self = static_cast<DemoMode *>(context);
  if (event.currentMode != AppMode::Demo)
  {
    return;
  }

  self->hardware->renderDemo(event.current, event.previous);
}

void DemoMode::handleDemoSerialTick(const DemoSerialTickEvent &event, void *context)
{
  auto *self = static_cast<DemoMode *>(context);
  self->printFrameToSerial(event.current);
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
