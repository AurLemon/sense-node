#pragma once

struct InferenceLabelMapEntry
{
  const char *modelLabel;
  const char *displayLabel;
};

const char *mapInferenceLabel(const char *rawLabel);
