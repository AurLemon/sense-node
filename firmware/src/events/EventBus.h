#pragma once

#include <Arduino.h>

#include "hardware/Hardware.h"
#include "inference/InteractionTypes.h"

enum class AppMode
{
  Demo,
  Capture
};

struct ModeChangedEvent
{
  AppMode previousMode;
  AppMode currentMode;
};

struct SensorFrameSampledEvent
{
  SensorFrame current;
  SensorFrame previous;
  AppMode currentMode;
};

struct ImuInferenceCompletedEvent
{
  bool ready = false;
  bool windowReady = false;
  MotionEvent motionEvent = MotionEvent::Reject;
  float confidence = 0.0f;
  char rawLabel[24]{};
  float accMag = 0.0f;
  float accDelta = 0.0f;
  float gyroMag = 0.0f;
  float motionEnergy = 0.0f;
  float peakAccDelta = 0.0f;
  unsigned long eventDurationMs = 0;
  bool quietDetected = false;
  bool activeDetected = false;
  bool impactDetected = false;
};

struct HandStateUpdatedEvent
{
  HandState handState = HandState::NoHand;
  uint16_t distanceMm = 0;
  bool tofValid = false;
  unsigned long stableMs = 0;
};

struct HandLeaveDetectedEvent
{
  HandState previousHandState = HandState::NoHand;
  uint16_t distanceMm = 0;
};

struct FusionDecisionEvent
{
  bool ready = false;
  FinalEvent finalEvent = FinalEvent::Unknown;
  HandState handState = HandState::NoHand;
  MotionEvent motionEvent = MotionEvent::Reject;
  float confidence = 0.0f;
  bool rejected = false;
  bool cooldownActive = false;
};

struct DemoSerialTickEvent
{
  SensorFrame current;
  ImuInferenceCompletedEvent imu;
  HandStateUpdatedEvent hand;
  FusionDecisionEvent fusion;
};

class EventBus
{
public:
  using ModeChangedHandler = void (*)(const ModeChangedEvent &, void *);
  using SensorFrameSampledHandler = void (*)(const SensorFrameSampledEvent &, void *);
  using DemoSerialTickHandler = void (*)(const DemoSerialTickEvent &, void *);
  using ImuInferenceCompletedHandler = void (*)(const ImuInferenceCompletedEvent &, void *);
  using HandStateUpdatedHandler = void (*)(const HandStateUpdatedEvent &, void *);
  using HandLeaveDetectedHandler = void (*)(const HandLeaveDetectedEvent &, void *);
  using FusionDecisionHandler = void (*)(const FusionDecisionEvent &, void *);

  void onModeChanged(ModeChangedHandler handler, void *context);
  void onSensorFrameSampled(SensorFrameSampledHandler handler, void *context);
  void onDemoSerialTick(DemoSerialTickHandler handler, void *context);
  void onImuInferenceCompleted(ImuInferenceCompletedHandler handler, void *context);
  void onHandStateUpdated(HandStateUpdatedHandler handler, void *context);
  void onHandLeaveDetected(HandLeaveDetectedHandler handler, void *context);
  void onFusionDecision(FusionDecisionHandler handler, void *context);

  void emitModeChanged(const ModeChangedEvent &event);
  void emitSensorFrameSampled(const SensorFrameSampledEvent &event);
  void emitDemoSerialTick(const DemoSerialTickEvent &event);
  void emitImuInferenceCompleted(const ImuInferenceCompletedEvent &event);
  void emitHandStateUpdated(const HandStateUpdatedEvent &event);
  void emitHandLeaveDetected(const HandLeaveDetectedEvent &event);
  void emitFusionDecision(const FusionDecisionEvent &event);

private:
  static constexpr uint8_t kMaxHandlers = 4;

  struct ModeChangedSubscription
  {
    ModeChangedHandler handler = nullptr;
    void *context = nullptr;
  };

  struct SensorFrameSampledSubscription
  {
    SensorFrameSampledHandler handler = nullptr;
    void *context = nullptr;
  };

  struct DemoSerialTickSubscription
  {
    DemoSerialTickHandler handler = nullptr;
    void *context = nullptr;
  };

  struct ImuInferenceCompletedSubscription
  {
    ImuInferenceCompletedHandler handler = nullptr;
    void *context = nullptr;
  };

  struct HandStateUpdatedSubscription
  {
    HandStateUpdatedHandler handler = nullptr;
    void *context = nullptr;
  };

  struct HandLeaveDetectedSubscription
  {
    HandLeaveDetectedHandler handler = nullptr;
    void *context = nullptr;
  };

  struct FusionDecisionSubscription
  {
    FusionDecisionHandler handler = nullptr;
    void *context = nullptr;
  };

  ModeChangedSubscription modeChangedHandlers[kMaxHandlers]{};
  SensorFrameSampledSubscription sensorFrameHandlers[kMaxHandlers]{};
  DemoSerialTickSubscription demoSerialHandlers[kMaxHandlers]{};
  ImuInferenceCompletedSubscription imuInferenceHandlers[kMaxHandlers]{};
  HandStateUpdatedSubscription handStateHandlers[kMaxHandlers]{};
  HandLeaveDetectedSubscription handLeaveHandlers[kMaxHandlers]{};
  FusionDecisionSubscription fusionHandlers[kMaxHandlers]{};
};
