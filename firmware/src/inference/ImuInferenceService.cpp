#include "inference/ImuInferenceService.h"

#include <cmath>
#include <cstring>

#include <sense_node_esp32s3_inferencing.h>

#include "inference/InteractionConfig.h"

namespace
{
  constexpr size_t kAxesPerFrame = EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;
  constexpr size_t kFrameCapacity = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
}

void ImuInferenceService::begin(EventBus &targetEventBus)
{
  eventBus = &targetEventBus;
  if (featureBuffer == nullptr)
  {
    featureBufferLength = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    featureBuffer = new float[featureBufferLength];
    accDeltaHistory = new float[kFrameCapacity];
  }

  reset();
  eventBus->onModeChanged(handleModeChanged, this);
  eventBus->onSensorFrameSampled(handleSensorFrameSampled, this);
}

const ImuInferenceCompletedEvent &ImuInferenceService::getLatestEvent() const
{
  return latestEvent;
}

void ImuInferenceService::handleModeChanged(const ModeChangedEvent &event, void *context)
{
  auto *self = static_cast<ImuInferenceService *>(context);
  if (event.currentMode != AppMode::Demo)
  {
    self->reset();
    return;
  }

  if (event.previousMode != event.currentMode)
  {
    self->reset();
    self->emitCurrentState();
  }
}

void ImuInferenceService::handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context)
{
  auto *self = static_cast<ImuInferenceService *>(context);
  if (event.currentMode != AppMode::Demo || !event.current.imuValid)
  {
    return;
  }

  self->appendFrame(event.current);
  self->runInference();
}

void ImuInferenceService::reset()
{
  framesCollected = 0;
  activeDurationMs = 0;
  if (featureBuffer != nullptr)
  {
    memset(featureBuffer, 0, sizeof(float) * featureBufferLength);
  }
  if (accDeltaHistory != nullptr)
  {
    memset(accDeltaHistory, 0, sizeof(float) * kFrameCapacity);
  }
  latestEvent = ImuInferenceCompletedEvent{};
  latestEvent.requiredFrames = static_cast<uint16_t>(kFrameCapacity);
  latestEvent.sampleIntervalMs = EI_CLASSIFIER_INTERVAL_MS;
  latestEvent.dspInputFrameSize = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
  latestEvent.nnInputFrameSize = EI_CLASSIFIER_NN_INPUT_FRAME_SIZE;
  latestEvent.classifierThreshold = EI_CLASSIFIER_THRESHOLD;
}

void ImuInferenceService::appendFrame(const SensorFrame &frame)
{
  if (framesCollected < kFrameCapacity)
  {
    framesCollected++;
  }
  else
  {
    memmove(featureBuffer, featureBuffer + kAxesPerFrame,
            sizeof(float) * (featureBufferLength - kAxesPerFrame));
    memmove(accDeltaHistory, accDeltaHistory + 1, sizeof(float) * (kFrameCapacity - 1));
  }

  const size_t offset = (framesCollected - 1) * kAxesPerFrame;
  const float ax = frame.accel.acceleration.x;
  const float ay = frame.accel.acceleration.y;
  const float az = frame.accel.acceleration.z;
  const float gx = frame.gyro.gyro.x;
  const float gy = frame.gyro.gyro.y;
  const float gz = frame.gyro.gyro.z;

  featureBuffer[offset + 0] = ax;
  featureBuffer[offset + 1] = ay;
  featureBuffer[offset + 2] = az;
  featureBuffer[offset + 3] = gx;
  featureBuffer[offset + 4] = gy;
  featureBuffer[offset + 5] = gz;

  const float accMag = std::sqrt(ax * ax + ay * ay + az * az);
  const float accDelta = std::fabs(accMag - InteractionConfig::GravityBaseline);
  const float gyroMag = std::sqrt(gx * gx + gy * gy + gz * gz);
  const float motionEnergy = accDelta + gyroMag * InteractionConfig::GyroEnergyWeight;

  latestEvent.accMag = accMag;
  latestEvent.accDelta = accDelta;
  latestEvent.gyroMag = gyroMag;
  latestEvent.motionEnergy = motionEnergy;
  latestEvent.quietDetected =
      accDelta < InteractionConfig::QuietAccDeltaThreshold &&
      gyroMag < InteractionConfig::QuietGyroThreshold;
  latestEvent.activeDetected =
      accDelta > InteractionConfig::ActiveAccDeltaThreshold ||
      gyroMag > InteractionConfig::ActiveGyroThreshold;
  latestEvent.impactDetected =
      accDelta > InteractionConfig::ImpactAccDeltaThreshold ||
      gyroMag > InteractionConfig::ImpactGyroThreshold;

  if (latestEvent.activeDetected)
  {
    activeDurationMs += EI_CLASSIFIER_INTERVAL_MS;
  }
  else
  {
    activeDurationMs = 0;
  }
  latestEvent.eventDurationMs = activeDurationMs;

  accDeltaHistory[framesCollected - 1] = accDelta;
  float peakAccDelta = 0.0f;
  for (size_t i = 0; i < framesCollected; ++i)
  {
    if (accDeltaHistory[i] > peakAccDelta)
    {
      peakAccDelta = accDeltaHistory[i];
    }
  }
  latestEvent.peakAccDelta = peakAccDelta;
}

void ImuInferenceService::emitCurrentState()
{
  if (eventBus != nullptr)
  {
    latestEvent.windowReady = framesCollected >= kFrameCapacity;
    latestEvent.collectedFrames = static_cast<uint16_t>(framesCollected);
    latestEvent.requiredFrames = static_cast<uint16_t>(kFrameCapacity);
    latestEvent.sampleIntervalMs = EI_CLASSIFIER_INTERVAL_MS;
    latestEvent.dspInputFrameSize = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    latestEvent.nnInputFrameSize = EI_CLASSIFIER_NN_INPUT_FRAME_SIZE;
    latestEvent.classifierThreshold = EI_CLASSIFIER_THRESHOLD;
    eventBus->emitImuInferenceCompleted(latestEvent);
  }
}

void ImuInferenceService::runInference()
{
  latestEvent.windowReady = framesCollected >= kFrameCapacity;
  if (!latestEvent.windowReady)
  {
    emitCurrentState();
    return;
  }

  signal_t signal;
  const int signalError =
      numpy::signal_from_buffer(featureBuffer, featureBufferLength, &signal);
  if (signalError != 0)
  {
    return;
  }

  ei_impulse_result_t result = {0};
  const EI_IMPULSE_ERROR inferenceError = run_classifier(&signal, &result, false);
  if (inferenceError != EI_IMPULSE_OK)
  {
    return;
  }

  size_t bestIndex = 0;
  float bestScore = result.classification[0].value;
  for (size_t i = 1; i < EI_CLASSIFIER_LABEL_COUNT; ++i)
  {
    if (result.classification[i].value > bestScore)
    {
      bestIndex = i;
      bestScore = result.classification[i].value;
    }
  }

  const char *rawLabel = result.classification[bestIndex].label;
  MotionEvent inferredMotion = motionEventFromLabel(rawLabel);
  if (bestScore < InteractionConfig::MinimumConfidence)
  {
    inferredMotion = MotionEvent::Reject;
  }
  else if (inferredMotion == MotionEvent::Tap && !latestEvent.impactDetected)
  {
    inferredMotion = MotionEvent::Reject;
  }

  latestEvent.ready = true;
  latestEvent.motionEvent = inferredMotion;
  latestEvent.confidence = bestScore;
  latestEvent.dspMs = result.timing.dsp;
  latestEvent.classificationMs = result.timing.classification;
  latestEvent.anomalyMs = result.timing.anomaly;
  latestEvent.postprocessingMs = result.timing.postprocessing;
  strncpy(latestEvent.rawLabel, rawLabel, sizeof(latestEvent.rawLabel) - 1);
  latestEvent.rawLabel[sizeof(latestEvent.rawLabel) - 1] = '\0';

  emitCurrentState();
}
