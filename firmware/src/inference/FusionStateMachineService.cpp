#include "inference/FusionStateMachineService.h"

#include "inference/InteractionConfig.h"

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

void FusionStateMachineService::handleImuInferenceCompleted(const ImuInferenceCompletedEvent &event, void *context)
{
  auto *self = static_cast<FusionStateMachineService *>(context);
  self->latestImu = event;
  self->evaluate();
}

void FusionStateMachineService::handleHandStateUpdated(const HandStateUpdatedEvent &event, void *context)
{
  auto *self = static_cast<FusionStateMachineService *>(context);
  self->latestHand = event;
  self->evaluate();
}

void FusionStateMachineService::handleHandLeaveDetected(const HandLeaveDetectedEvent &event, void *context)
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
  latestFusion.motionEvent = MotionEvent::Reject;
  latestFusion.finalEvent = FinalEvent::Unknown;
  handLeavePending = false;
  lastTapMs = 0;
  lastBoardMotionMs = 0;
}

void FusionStateMachineService::evaluate()
{
  const unsigned long now = millis();

  latestFusion.handState = latestHand.handState;
  latestFusion.motionEvent = latestImu.motionEvent;
  latestFusion.confidence = latestImu.confidence;
  latestFusion.cooldownActive =
      lastTapMs != 0 && (now - lastTapMs) < InteractionConfig::TapCooldownMs;
  latestFusion.rejected = latestImu.ready && latestImu.motionEvent == MotionEvent::Reject;

  if (!latestImu.ready || !latestImu.windowReady)
  {
    latestFusion.ready = false;
    latestFusion.finalEvent = FinalEvent::Unknown;
    emitCurrentState();
    return;
  }

  latestFusion.ready = true;

  const bool boardMotionDetected =
      latestImu.motionEvent == MotionEvent::BoardMotion &&
      latestImu.eventDurationMs >= InteractionConfig::BoardMotionMinimumMs;
  if (boardMotionDetected)
  {
    lastBoardMotionMs = now;
  }
  const bool boardMotionActive =
      lastBoardMotionMs != 0 && (now - lastBoardMotionMs) < InteractionConfig::BoardMotionHoldMs;

  if (latestImu.motionEvent == MotionEvent::Tap && latestImu.impactDetected &&
      !latestFusion.cooldownActive)
  {
    lastTapMs = now;
    latestFusion.cooldownActive = true;
    latestFusion.rejected = false;
    latestFusion.finalEvent = FinalEvent::Tap;
    emitCurrentState();
    return;
  }

  if (boardMotionActive)
  {
    latestFusion.rejected = false;
    latestFusion.finalEvent = FinalEvent::BoardMotion;
    emitCurrentState();
    return;
  }

  if (handLeavePending)
  {
    handLeavePending = false;
    latestFusion.rejected = false;
    latestFusion.finalEvent = FinalEvent::HandLeave;
    emitCurrentState();
    return;
  }

  if (latestHand.handState == HandState::Near)
  {
    latestFusion.rejected = false;
    latestFusion.finalEvent = FinalEvent::HandNear;
    emitCurrentState();
    return;
  }

  if (latestHand.handState == HandState::Hover)
  {
    latestFusion.rejected = false;
    latestFusion.finalEvent = FinalEvent::HandHover;
    emitCurrentState();
    return;
  }

  if (latestFusion.rejected)
  {
    latestFusion.finalEvent = FinalEvent::Unknown;
    emitCurrentState();
    return;
  }

  latestFusion.finalEvent = FinalEvent::Idle;
  emitCurrentState();
}

void FusionStateMachineService::emitCurrentState()
{
  if (eventBus != nullptr)
  {
    eventBus->emitFusionDecision(latestFusion);
  }
}
