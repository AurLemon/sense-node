#pragma once

#include "inference/InteractionTypes.h"

const char *resolveExpressionFace(FinalEvent finalEvent);
const char *resolveExpressionFace(const char *eventLabel);
const char *resolveRuntimeExpressionFace(FinalEvent finalEvent, unsigned long now);
const char *resolveRuntimeExpressionFace(const char *eventLabel, unsigned long now);
