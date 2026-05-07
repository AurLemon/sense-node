#include "display/ExpressionScheme.h"

#include <Arduino.h>

#include <cstring>

namespace
{
  constexpr const char *kIdleFaces[] = {"OmO", "OwO", "UwU", "^w^", "oWo", "ovo", "uWu", "n_n"};
  constexpr size_t kIdleFaceCount = sizeof(kIdleFaces) / sizeof(kIdleFaces[0]);
  constexpr unsigned long kIdleFaceRotateMs = 4200;
  constexpr const char *kTapFace = "o_o";
  constexpr const char *kBoardMotionFace = ">_<";
  size_t currentIdleFaceIndex = 0;
  unsigned long nextIdleFaceRotateMs = 0;

  bool isEventLabel(const char *label, const char *expected)
  {
    return label != nullptr && std::strcmp(label, expected) == 0;
  }

  const char *resolveIdleFace()
  {
    return kIdleFaces[currentIdleFaceIndex];
  }

  const char *resolveRuntimeIdleFace(unsigned long now)
  {
    if (nextIdleFaceRotateMs == 0)
    {
      nextIdleFaceRotateMs = now + kIdleFaceRotateMs;
      return resolveIdleFace();
    }

    if (now >= nextIdleFaceRotateMs)
    {
      const long offset = random(1, static_cast<long>(kIdleFaceCount));
      currentIdleFaceIndex = (currentIdleFaceIndex + static_cast<size_t>(offset)) % kIdleFaceCount;
      nextIdleFaceRotateMs = now + kIdleFaceRotateMs;
    }

    return resolveIdleFace();
  }
}

const char *resolveExpressionFace(FinalEvent finalEvent)
{
  switch (finalEvent)
  {
  case FinalEvent::Idle:
  case FinalEvent::Reject:
  case FinalEvent::Unknown:
    return resolveIdleFace();
  case FinalEvent::Tap:
    return kTapFace;
  case FinalEvent::BoardMotion:
    return kBoardMotionFace;
  }

    return resolveIdleFace();
}

const char *resolveExpressionFace(const char *eventLabel)
{
  if (isEventLabel(eventLabel, "tap"))
  {
    return kTapFace;
  }
  if (isEventLabel(eventLabel, "board_motion"))
  {
    return kBoardMotionFace;
  }
  if (isEventLabel(eventLabel, "idle") || isEventLabel(eventLabel, "reject") ||
      isEventLabel(eventLabel, "unknown"))
  {
    return resolveIdleFace();
  }

  return nullptr;
}

const char *resolveRuntimeExpressionFace(FinalEvent finalEvent, unsigned long now)
{
  switch (finalEvent)
  {
  case FinalEvent::Idle:
  case FinalEvent::Reject:
  case FinalEvent::Unknown:
    return resolveRuntimeIdleFace(now);
  case FinalEvent::Tap:
    return kTapFace;
  case FinalEvent::BoardMotion:
    return kBoardMotionFace;
  }

  return resolveRuntimeIdleFace(now);
}

const char *resolveRuntimeExpressionFace(const char *eventLabel, unsigned long now)
{
  if (isEventLabel(eventLabel, "tap"))
  {
    return kTapFace;
  }
  if (isEventLabel(eventLabel, "board_motion"))
  {
    return kBoardMotionFace;
  }
  if (isEventLabel(eventLabel, "idle") || isEventLabel(eventLabel, "reject") ||
      isEventLabel(eventLabel, "unknown"))
  {
    return resolveRuntimeIdleFace(now);
  }

  return nullptr;
}
