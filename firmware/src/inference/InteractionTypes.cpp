#include "inference/InteractionTypes.h"

#include <cstring>

const char *toString(HandState state)
{
  switch (state)
  {
  case HandState::NoHand:
    return "no_hand";
  case HandState::Hover:
    return "hand_hover";
  case HandState::Near:
    return "hand_near";
  }

  return "no_hand";
}

const char *toString(MotionEvent event)
{
  switch (event)
  {
  case MotionEvent::Idle:
    return "idle";
  case MotionEvent::Tap:
    return "tap";
  case MotionEvent::BoardMotion:
    return "board_motion";
  case MotionEvent::Unknown:
    return "unknown";
  case MotionEvent::Reject:
    return "reject";
  }

  return "reject";
}

const char *toString(FinalEvent event)
{
  switch (event)
  {
  case FinalEvent::Idle:
    return "idle";
  case FinalEvent::Tap:
    return "tap";
  case FinalEvent::BoardMotion:
    return "board_motion";
  case FinalEvent::Reject:
    return "reject";
  case FinalEvent::Unknown:
    return "unknown";
  }

  return "unknown";
}

const char *toString(FusionState state)
{
  switch (state)
  {
  case FusionState::Idle:
    return "idle";
  case FusionState::TapPulse:
    return "tap_pulse";
  case FusionState::TapCooldown:
    return "tap_cooldown";
  case FusionState::BoardMotion:
    return "board_motion";
  case FusionState::Reject:
    return "reject";
  }

  return "reject";
}

MotionEvent motionEventFromLabel(const char *rawLabel)
{
  if (rawLabel == nullptr)
  {
    return MotionEvent::Unknown;
  }

  if (std::strcmp(rawLabel, "idle") == 0)
  {
    return MotionEvent::Idle;
  }

  if (std::strcmp(rawLabel, "tap") == 0)
  {
    return MotionEvent::Tap;
  }

  if (std::strcmp(rawLabel, "board_motion") == 0)
  {
    return MotionEvent::BoardMotion;
  }

  return MotionEvent::Unknown;
}
