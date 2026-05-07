#include "modes/Modes.h"

#include <esp_heap_caps.h>
#include <esp_system.h>

#include "display/ExpressionScheme.h"

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
  self->warmupInfoPrinted = false;
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
  if (!self->warmupInfoPrinted && (!event.imu.ready || !event.imu.windowReady))
  {
    self->printWarmupInfo(event);
    self->warmupInfoPrinted = true;
  }
  self->printDemoJsonLine(event);
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

void DemoMode::printWarmupInfo(const DemoSerialTickEvent &event)
{
  const uint32_t cpuMhz = getCpuFrequencyMhz();
  const uint32_t freeHeap = esp_get_free_heap_size();
  const uint32_t minFreeHeap = esp_get_minimum_free_heap_size();
  const uint32_t maxAllocHeap = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  const uint16_t requiredFrames = event.imu.requiredFrames;
  const uint16_t collectedFrames = event.imu.collectedFrames;
  const uint16_t remainingFrames =
      collectedFrames < requiredFrames ? requiredFrames - collectedFrames : 0;
  const unsigned long warmupTargetMs =
      static_cast<unsigned long>(requiredFrames) * event.imu.sampleIntervalMs;
  const unsigned long remainingMs =
      static_cast<unsigned long>(remainingFrames) * event.imu.sampleIntervalMs;

  Serial.print("{\"type\":\"warmup_info\",\"sample_rate_hz\":");
  Serial.print(event.imu.sampleIntervalMs > 0 ? 1000 / event.imu.sampleIntervalMs : 0);
  Serial.print(",\"window_frames\":");
  Serial.print(requiredFrames);
  Serial.print(",\"window_fill\":\"");
  Serial.print(collectedFrames);
  Serial.print('/');
  Serial.print(requiredFrames);
  Serial.print("\",\"warmup_target_ms\":");
  Serial.print(warmupTargetMs);
  Serial.print(",\"warmup_remaining_ms\":");
  Serial.print(remainingMs);
  Serial.print(",\"model_labels\":[\"idle\",\"tap\",\"board_motion\"],\"dsp_input_frame_size\":");
  Serial.print(event.imu.dspInputFrameSize);
  Serial.print(",\"nn_input_frame_size\":");
  Serial.print(event.imu.nnInputFrameSize);
  Serial.print(",\"classifier_threshold\":");
  Serial.print(event.imu.classifierThreshold, 4);
  Serial.print(",\"cpu_mhz\":");
  Serial.print(cpuMhz);
  Serial.print(",\"free_heap\":");
  Serial.print(freeHeap);
  Serial.print(",\"min_free_heap\":");
  Serial.print(minFreeHeap);
  Serial.print(",\"max_alloc_heap\":");
  Serial.print(maxAllocHeap);
  Serial.println('}');
}

void DemoMode::printDemoJsonLine(const DemoSerialTickEvent &event)
{
  const SensorFrame &frame = event.current;
  const unsigned long uptimeMs = millis();
  const uint32_t cpuMhz = getCpuFrequencyMhz();
  const uint32_t freeHeap = esp_get_free_heap_size();
  const uint32_t minFreeHeap = esp_get_minimum_free_heap_size();
  const uint32_t maxAllocHeap = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  const unsigned long inferenceMs =
      event.imu.dspMs + event.imu.classificationMs + event.imu.postprocessingMs;

  Serial.print("{\"type\":\"sensor_frame\",\"tof_mm\":");
  if (frame.tofValid)
  {
    Serial.print(frame.tofMm);
  }
  else
  {
    Serial.print(-1);
  }

  Serial.print(",\"accel\":");

  if (frame.imuValid)
  {
    Serial.print("{\"x\":");
    Serial.print(frame.accel.acceleration.x, 4);
    Serial.print(",\"y\":");
    Serial.print(frame.accel.acceleration.y, 4);
    Serial.print(",\"z\":");
    Serial.print(frame.accel.acceleration.z, 4);
    Serial.print("},\"gyro\":{\"x\":");
    Serial.print(frame.gyro.gyro.x, 4);
    Serial.print(",\"y\":");
    Serial.print(frame.gyro.gyro.y, 4);
    Serial.print(",\"z\":");
    Serial.print(frame.gyro.gyro.z, 4);
    Serial.print('}');
  }
  else
  {
    Serial.print("{\"x\":0,\"y\":0,\"z\":0},\"gyro\":{\"x\":0,\"y\":0,\"z\":0}");
  }

  Serial.print(",\"uptime_ms\":");
  Serial.print(uptimeMs);
  Serial.print(",\"imu_label\":\"");
  Serial.print(event.imu.ready && event.imu.windowReady ? event.imu.rawMlLabel : "warming_up");
  Serial.print("\",\"confidence\":");
  Serial.print(event.imu.ready ? event.imu.confidence : 0.0f, 4);
  Serial.print(",\"hand_state\":\"");
  Serial.print(toString(event.hand.handState));
  Serial.print("\",\"motion_event\":\"");
  Serial.print(toString(event.imu.motionEvent));
  Serial.print("\",\"fusion_state\":\"");
  Serial.print(toString(event.fusion.fusionState));
  Serial.print("\",\"final_event\":\"");
  Serial.print(event.fusion.ready ? toString(event.fusion.finalEvent) : "warming_up");
  Serial.print("\",\"display_face\":\"");
  const char *displayFace =
      event.fusion.ready ? resolveRuntimeExpressionFace(event.fusion.finalEvent, millis()) : nullptr;
  if (displayFace != nullptr)
  {
    Serial.print(displayFace);
  }
  Serial.print("\",\"acc_mag\":");
  Serial.print(event.imu.accMag, 4);
  Serial.print(",\"acc_delta\":");
  Serial.print(event.imu.accDelta, 4);
  Serial.print(",\"acc_jerk\":");
  Serial.print(event.imu.accJerk, 4);
  Serial.print(",\"gyro_mag\":");
  Serial.print(event.imu.gyroMag, 4);
  Serial.print(",\"tap_candidate\":");
  Serial.print(event.fusion.tapCandidate ? "1" : "0");
  Serial.print(",\"raw_motion_active\":");
  Serial.print(event.fusion.rawMotionActive ? "1" : "0");
  Serial.print(",\"raw_stable\":");
  Serial.print(event.fusion.rawStable ? "1" : "0");
  Serial.print(",\"stable_duration_ms\":");
  Serial.print(event.fusion.stableDurationMs);
  Serial.print(",\"motion_age_ms\":");
  Serial.print(event.fusion.motionAgeMs);
  Serial.print(",\"tap_cooldown_remaining_ms\":");
  Serial.print(event.fusion.tapCooldownRemainingMs);
  Serial.print(",\"inference_ms\":");
  Serial.print(inferenceMs);
  Serial.print(",\"dsp_ms\":");
  Serial.print(event.imu.dspMs);
  Serial.print(",\"classification_ms\":");
  Serial.print(event.imu.classificationMs);
  Serial.print(",\"postprocessing_ms\":");
  Serial.print(event.imu.postprocessingMs);
  Serial.print(",\"window_fill\":\"");
  Serial.print(event.imu.collectedFrames);
  Serial.print('/');
  Serial.print(event.imu.requiredFrames);
  Serial.print("\",\"cpu_mhz\":");
  Serial.print(cpuMhz);
  Serial.print(",\"free_heap\":");
  Serial.print(freeHeap);
  Serial.print(",\"min_free_heap\":");
  Serial.print(minFreeHeap);
  Serial.print(",\"max_alloc_heap\":");
  Serial.print(maxAllocHeap);
  Serial.println('}');
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
