#pragma once

enum class HandState
{
  NoHand,
  Hover,
  Near
};

enum class MotionEvent
{
  Idle,
  Tap,
  BoardMotion,
  Reject
};

enum class FinalEvent
{
  Idle,
  HandHover,
  HandNear,
  HandLeave,
  Tap,
  BoardMotion,
  Unknown
};

const char *toString(HandState state);
const char *toString(MotionEvent event);
const char *toString(FinalEvent event);
MotionEvent motionEventFromLabel(const char *rawLabel);
