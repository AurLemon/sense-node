#include "inference/FusionStateMachineService.h"

#include <cstring>

#include "inference/InteractionConfig.h"

namespace
{
  bool isRawLabel(const char *label, const char *expected)
  {
    return label != nullptr && std::strcmp(label, expected) == 0;
  }
}

void FusionStateMachineService::begin(EventBus &targetEventBus)
{
  eventBus = &targetEventBus;
  reset();
  eventBus->onModeChanged(handleModeChanged, this);
  eventBus->onImuInferenceCompleted(handleImuInferenceCompleted, this);
  eventBus->onHandStateUpdated(handleHandStateUpdated, this);
  eventBus->onHandLeaveDetected(handleHandLeaveDetected, this);
}

const FusionDecisionEvent &FusionStateMachineService::getLatestEvent() const
{
  return latestFusion;
}

void FusionStateMachineService::handleModeChanged(const ModeChangedEvent &event, void *context)
{
  auto *self = static_cast<FusionStateMachineService *>(context);
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

void FusionStateMachineService::handleImuInferenceCompleted(const ImuInferenceCompletedEvent &event,
                                                            void *context)
{
  auto *self = static_cast<FusionStateMachineService *>(context);
  self->latestImu = event;
  self->evaluate();
}

void FusionStateMachineService::handleHandStateUpdated(const HandStateUpdatedEvent &event,
                                                       void *context)
{
  auto *self = static_cast<FusionStateMachineService *>(context);
  self->latestHand = event;
  self->evaluate();
}

void FusionStateMachineService::handleHandLeaveDetected(const HandLeaveDetectedEvent &event,
                                                        void *context)
{
  auto *self = static_cast<FusionStateMachineService *>(context);
  (void)event;
  self->handLeavePending = true;
  self->evaluate();
}

void FusionStateMachineService::reset()
{
  latestImu = ImuInferenceCompletedEvent{};
  latestHand = HandStateUpdatedEvent{};
  latestHand.handState = HandState::NoHand;
  latestFusion = FusionDecisionEvent{};
  latestFusion.handState = HandState::NoHand;
  latestFusion.motionEvent = MotionEvent::Unknown;
  latestFusion.finalEvent = FinalEvent::Unknown;
  latestFusion.fusionState = FusionState::Idle;
  handLeavePending = false;
  fusionState = FusionState::Idle;
  tapPulseUntilMs = 0;
  tapCooldownUntilMs = 0;
  motionStartMs = 0;
}

void FusionStateMachineService::enterTapPulse(unsigned long now)
{
  fusionState = FusionState::TapPulse;
  tapPulseUntilMs = now + InteractionConfig::TapPulseMs;
  tapCooldownUntilMs = tapPulseUntilMs + InteractionConfig::TapCooldownMs;
  motionStartMs = 0;
}

void FusionStateMachineService::enterBoardMotion(unsigned long now)
{
  fusionState = FusionState::BoardMotion;
  motionStartMs = latestImu.lastRawMotionMs != 0 ? latestImu.lastRawMotionMs : now;
}

bool FusionStateMachineService::rawStableFor(unsigned long now, unsigned long durationMs) const
{
  (void)now;
  return latestImu.rawStable && latestImu.stableDurationMs >= durationMs;
}

void FusionStateMachineService::evaluate()
{
  const unsigned long now = millis();

  latestFusion.handState = latestHand.handState;
  latestFusion.motionEvent = latestImu.rawMlEvent;
  latestFusion.confidence = latestImu.confidence;
  latestFusion.cooldownActive = now < tapCooldownUntilMs;
  latestFusion.tapCooldownRemainingMs = latestFusion.cooldownActive ? tapCooldownUntilMs - now : 0;
  latestFusion.rawMotionActive = latestImu.rawMotionActive;
  latestFusion.rawStable = latestImu.rawStable;
  latestFusion.motionAgeMs = latestImu.motionAgeMs;
  latestFusion.stableDurationMs = latestImu.stableDurationMs;
  latestFusion.tapCandidate = false;
  latestFusion.rejected = false;

  if (!latestImu.ready || !latestImu.windowReady)
  {
    latestFusion.ready = false;
    latestFusion.fusionState = FusionState::Idle;
    latestFusion.finalEvent = FinalEvent::Unknown;
    emitCurrentState();
    return;
  }

  latestFusion.ready = true;

  const bool modelTapCandidate =
      isRawLabel(latestImu.rawMlLabel, "tap") && latestImu.confidence >= 0.70f;
  const bool impactTapCandidate =
      (latestImu.peakAccJerk >= InteractionConfig::TapJerkThreshold ||
       latestImu.peakAccDelta >= InteractionConfig::TapDeltaThreshold) &&
      latestImu.peakGyroMag < InteractionConfig::TapGyroMax;
  const bool tapCandidate = modelTapCandidate || impactTapCandidate;
  latestFusion.tapCandidate = tapCandidate;

  const bool modelBoardMotionCandidate =
      isRawLabel(latestImu.rawMlLabel, "board_motion") &&
      latestImu.confidence >= 0.75f;
  const bool rawMotionCandidate =
      latestImu.rawMotionActive && latestImu.motionAgeMs >= InteractionConfig::BoardMotionMinimumMs;
  const bool rawStable = latestImu.rawStable;

  if (fusionState == FusionState::TapPulse)
  {
    latestFusion.finalEvent = FinalEvent::Tap;
    latestFusion.fusionState = fusionState;
    latestFusion.rejected = false;

    if (now >= tapPulseUntilMs)
    {
      fusionState = FusionState::TapCooldown;
      latestFusion.fusionState = fusionState;
    }

    emitCurrentState();
    return;
  }

  if (fusionState == FusionState::TapCooldown)
  {
    latestFusion.finalEvent = rawStable ? FinalEvent::Idle : FinalEvent::Reject;
    latestFusion.fusionState = fusionState;
    latestFusion.rejected = !rawStable;

    if (now < tapCooldownUntilMs)
    {
      emitCurrentState();
      return;
    }

    fusionState = rawStable ? FusionState::Idle : FusionState::Reject;
  }

  if (fusionState == FusionState::BoardMotion)
  {
    if (tapCandidate && now >= tapCooldownUntilMs)
    {
      enterTapPulse(now);
      latestFusion.finalEvent = FinalEvent::Tap;
      latestFusion.fusionState = fusionState;
      latestFusion.rejected = false;
      latestFusion.cooldownActive = true;
      latestFusion.tapCooldownRemainingMs = tapCooldownUntilMs - now;
      emitCurrentState();
      return;
    }

    if (rawStableFor(now, InteractionConfig::MotionSettleMs))
    {
      fusionState = FusionState::Idle;
      latestFusion.finalEvent = FinalEvent::Idle;
      latestFusion.fusionState = fusionState;
      latestFusion.rejected = false;
      emitCurrentState();
      return;
    }

    if (motionStartMs != 0 && now - motionStartMs > InteractionConfig::MaxMotionHoldMs)
    {
      fusionState = rawStable ? FusionState::Idle : FusionState::Reject;
      latestFusion.finalEvent = rawStable ? FinalEvent::Idle : FinalEvent::Reject;
      latestFusion.fusionState = fusionState;
      latestFusion.rejected = !rawStable;
      emitCurrentState();
      return;
    }

    latestFusion.finalEvent = FinalEvent::BoardMotion;
    latestFusion.fusionState = fusionState;
    latestFusion.rejected = false;
    emitCurrentState();
    return;
  }

  if (tapCandidate && now >= tapCooldownUntilMs)
  {
    enterTapPulse(now);
    latestFusion.finalEvent = FinalEvent::Tap;
    latestFusion.fusionState = fusionState;
    latestFusion.rejected = false;
    latestFusion.cooldownActive = true;
    latestFusion.tapCooldownRemainingMs = tapCooldownUntilMs - now;
    emitCurrentState();
    return;
  }

  if (modelBoardMotionCandidate || rawMotionCandidate)
  {
    enterBoardMotion(now);
    latestFusion.finalEvent = FinalEvent::BoardMotion;
    latestFusion.fusionState = fusionState;
    latestFusion.rejected = false;
    emitCurrentState();
    return;
  }

  if (rawStable)
  {
    fusionState = FusionState::Idle;
    latestFusion.finalEvent = FinalEvent::Idle;
    latestFusion.fusionState = fusionState;
    latestFusion.rejected = false;
    emitCurrentState();
    return;
  }

  fusionState = FusionState::Reject;
  latestFusion.finalEvent = FinalEvent::Reject;
  latestFusion.fusionState = fusionState;
  latestFusion.rejected = true;

  if (handLeavePending)
  {
    handLeavePending = false;
  }

  emitCurrentState();
}

void FusionStateMachineService::emitCurrentState()
{
  if (eventBus != nullptr)
  {
    eventBus->emitFusionDecision(latestFusion);
  }
}
