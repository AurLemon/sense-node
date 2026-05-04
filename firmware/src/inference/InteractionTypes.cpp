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
  case FinalEvent::HandHover:
    return "hand_hover";
  case FinalEvent::HandNear:
    return "hand_near";
  case FinalEvent::HandLeave:
    return "hand_leave";
  case FinalEvent::Tap:
    return "tap";
  case FinalEvent::BoardMotion:
    return "board_motion";
  case FinalEvent::Unknown:
    return "unknown";
  }

  return "unknown";
}

MotionEvent motionEventFromLabel(const char *rawLabel)
{
  if (rawLabel == nullptr)
  {
    return MotionEvent::Reject;
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

  return MotionEvent::Reject;
}
