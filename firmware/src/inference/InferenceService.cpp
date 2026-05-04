#include "inference/InferenceService.h"
#include "inference/InferenceLabelMap.h"

#include <sense_node_esp32s3_inferencing.h>

void InferenceService::begin(EventBus &targetEventBus)
{
  eventBus = &targetEventBus;
  if (featureBuffer == nullptr)
  {
    featureBufferLength = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    featureBuffer = new float[featureBufferLength];
  }
  reset();
  eventBus->onModeChanged(handleModeChanged, this);
  eventBus->onSensorFrameSampled(handleSensorFrameSampled, this);
}

const InferenceCompletedEvent &InferenceService::getLatestEvent() const
{
  return latestEvent;
}

void InferenceService::handleModeChanged(const ModeChangedEvent &event, void *context)
{
  auto *self = static_cast<InferenceService *>(context);
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

void InferenceService::handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context)
{
  auto *self = static_cast<InferenceService *>(context);
  if (event.currentMode != AppMode::Demo)
  {
    return;
  }

  if (!event.current.tofValid || !event.current.imuValid)
  {
    return;
  }

  self->appendFrame(event.current);
  self->runInference();
}

void InferenceService::reset()
{
  framesCollected = 0;
  if (featureBuffer != nullptr)
  {
    memset(featureBuffer, 0, sizeof(float) * featureBufferLength);
  }
  latestEvent = InferenceCompletedEvent{};
}

void InferenceService::appendFrame(const SensorFrame &frame)
{
  constexpr size_t kAxesPerFrame = EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;
  constexpr size_t kFrameCapacity = EI_CLASSIFIER_RAW_SAMPLE_COUNT;

  if (framesCollected < kFrameCapacity)
  {
    framesCollected++;
  }
  else
  {
    memmove(featureBuffer, featureBuffer + kAxesPerFrame,
            sizeof(float) * (featureBufferLength - kAxesPerFrame));
  }

  const size_t offset = (framesCollected - 1) * kAxesPerFrame;
  featureBuffer[offset + 0] = static_cast<float>(frame.tofMm);
  featureBuffer[offset + 1] = frame.accel.acceleration.x;
  featureBuffer[offset + 2] = frame.accel.acceleration.y;
  featureBuffer[offset + 3] = frame.accel.acceleration.z;
  featureBuffer[offset + 4] = frame.gyro.gyro.x;
  featureBuffer[offset + 5] = frame.gyro.gyro.y;
  featureBuffer[offset + 6] = frame.gyro.gyro.z;
}

void InferenceService::emitCurrentState()
{
  if (eventBus != nullptr)
  {
    latestEvent.sensorWindowReady = framesCollected >= EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    eventBus->emitInferenceCompleted(latestEvent);
  }
}

void InferenceService::runInference()
{
  latestEvent.sensorWindowReady = framesCollected >= EI_CLASSIFIER_RAW_SAMPLE_COUNT;
  if (!latestEvent.sensorWindowReady)
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
  const char *displayLabel = mapInferenceLabel(rawLabel);

  latestEvent.ready = true;
  latestEvent.confidence = bestScore;
  strncpy(latestEvent.rawLabel, rawLabel, sizeof(latestEvent.rawLabel) - 1);
  latestEvent.rawLabel[sizeof(latestEvent.rawLabel) - 1] = '\0';
  strncpy(latestEvent.displayLabel, displayLabel, sizeof(latestEvent.displayLabel) - 1);
  latestEvent.displayLabel[sizeof(latestEvent.displayLabel) - 1] = '\0';

  emitCurrentState();
}
