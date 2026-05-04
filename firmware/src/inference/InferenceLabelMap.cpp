#include "inference/InferenceLabelMap.h"

#include <cstring>

namespace
{
  constexpr InferenceLabelMapEntry kLabelMap[] = {
      {"idle", "idle"},
      {"light_tap", "light_tap"},
      {"hard_tap", "hard_tap"},
      {"board_flick", "board_flick"},
      {"board_motion", "board_motion"},
      {"board_pickup", "board_pickup"},
      {"board_putdown", "board_putdown"},
      {"unknown_noise", "unknown_noise"},
      {"hand_near", "hand_near"},
      {"hand_hover", "hand_hover"},
      {"hand_leave", "hand_leave"},
  };
}

const char *mapInferenceLabel(const char *rawLabel)
{
  for (const auto &entry : kLabelMap)
  {
    if (std::strcmp(entry.modelLabel, rawLabel) == 0)
    {
      return entry.displayLabel;
    }
  }

  return rawLabel;
}
