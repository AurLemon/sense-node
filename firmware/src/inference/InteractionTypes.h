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
  Unknown,
  Reject
};

enum class FinalEvent
{
  Idle,
  Tap,
  BoardMotion,
  Reject,
  Unknown
};

enum class FusionState
{
  Idle,
  TapPulse,
  TapCooldown,
  BoardMotion,
  Reject
};

const char *toString(HandState state);
const char *toString(MotionEvent event);
const char *toString(FinalEvent event);
const char *toString(FusionState state);
MotionEvent motionEventFromLabel(const char *rawLabel);
