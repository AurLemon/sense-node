#include "inference/TofHandStateService.h"

#include "inference/InteractionConfig.h"

void TofHandStateService::begin(EventBus &targetEventBus)
{
  eventBus = &targetEventBus;
  reset();
  eventBus->onModeChanged(handleModeChanged, this);
  eventBus->onSensorFrameSampled(handleSensorFrameSampled, this);
}

const HandStateUpdatedEvent &TofHandStateService::getLatestEvent() const
{
  return latestEvent;
}

void TofHandStateService::handleModeChanged(const ModeChangedEvent &event, void *context)
{
  auto *self = static_cast<TofHandStateService *>(context);
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

void TofHandStateService::handleSensorFrameSampled(const SensorFrameSampledEvent &event, void *context)
{
  auto *self = static_cast<TofHandStateService *>(context);
  if (event.currentMode != AppMode::Demo)
  {
    return;
  }

  self->update(event.current.tofMm, event.current.tofValid);
}

void TofHandStateService::reset()
{
  currentState = HandState::NoHand;
  candidateState = HandState::NoHand;
  candidateSinceMs = 0;
  stateSinceMs = millis();
  latestEvent = HandStateUpdatedEvent{};
  latestEvent.handState = HandState::NoHand;
}

HandState TofHandStateService::classifyDesiredState(uint16_t distanceMm, bool tofValid) const
{
  const bool valid = tofValid && distanceMm != InteractionConfig::TofInvalidMm;
  if (!valid)
  {
    return HandState::NoHand;
  }

  switch (currentState)
  {
  case HandState::NoHand:
    if (distanceMm < InteractionConfig::NearEnterMm)
    {
      return HandState::Near;
    }
    if (distanceMm >= InteractionConfig::HoverMinMm &&
        distanceMm <= InteractionConfig::HoverMaxEnterMm)
    {
      return HandState::Hover;
    }
    return HandState::NoHand;

  case HandState::Near:
    if (distanceMm > InteractionConfig::HoverMaxExitMm)
    {
      return HandState::NoHand;
    }
    if (distanceMm > InteractionConfig::NearExitMm &&
        distanceMm <= InteractionConfig::HoverMaxEnterMm)
    {
      return HandState::Hover;
    }
    return HandState::Near;

  case HandState::Hover:
    if (distanceMm > InteractionConfig::HoverMaxExitMm)
    {
      return HandState::NoHand;
    }
    if (distanceMm < InteractionConfig::NearEnterMm)
    {
      return HandState::Near;
    }
    return HandState::Hover;
  }

  return HandState::NoHand;
}

void TofHandStateService::update(uint16_t distanceMm, bool tofValid)
{
  const unsigned long now = millis();
  const HandState desiredState = classifyDesiredState(distanceMm, tofValid);

  if (desiredState != currentState)
  {
    if (candidateState != desiredState)
    {
      candidateState = desiredState;
      candidateSinceMs = now;
    }

    const unsigned long requiredStableMs =
        desiredState == HandState::NoHand ? InteractionConfig::LeaveStableMs
                                          : InteractionConfig::HandStableMs;
    if (now - candidateSinceMs >= requiredStableMs)
    {
      const HandState previousState = currentState;
      currentState = desiredState;
      stateSinceMs = now;
      candidateState = currentState;
      candidateSinceMs = now;

      if (currentState == HandState::NoHand && previousState != HandState::NoHand &&
          eventBus != nullptr)
      {
        HandLeaveDetectedEvent leaveEvent{};
        leaveEvent.previousHandState = previousState;
        leaveEvent.distanceMm = distanceMm;
        eventBus->emitHandLeaveDetected(leaveEvent);
      }
    }
  }
  else
  {
    candidateState = currentState;
    candidateSinceMs = now;
  }

  latestEvent.handState = currentState;
  latestEvent.distanceMm = distanceMm;
  latestEvent.tofValid = tofValid;
  latestEvent.stableMs = now - stateSinceMs;

  emitCurrentState();
}

void TofHandStateService::emitCurrentState()
{
  if (eventBus != nullptr)
  {
    eventBus->emitHandStateUpdated(latestEvent);
  }
}
