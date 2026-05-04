#pragma once

#include <Arduino.h>

struct InteractionConfig
{
  static constexpr uint16_t TofInvalidMm = 8191;

  static constexpr uint16_t NearEnterMm = 80;
  static constexpr uint16_t NearExitMm = 120;
  static constexpr uint16_t HoverMinMm = 80;
  static constexpr uint16_t HoverMaxEnterMm = 350;
  static constexpr uint16_t HoverMaxExitMm = 420;

  static constexpr unsigned long HandStableMs = 250;
  static constexpr unsigned long LeaveStableMs = 300;

  static constexpr float GravityBaseline = 9.8f;
  static constexpr float GyroEnergyWeight = 0.5f;
  static constexpr float QuietAccDeltaThreshold = 0.5f;
  static constexpr float QuietGyroThreshold = 0.15f;
  static constexpr float ActiveAccDeltaThreshold = 1.5f;
  static constexpr float ActiveGyroThreshold = 0.4f;
  static constexpr float ImpactAccDeltaThreshold = 3.0f;
  static constexpr float ImpactGyroThreshold = 0.8f;

  static constexpr float MinimumConfidence = 0.6f;
  static constexpr unsigned long TapCooldownMs = 400;
  static constexpr unsigned long BoardMotionHoldMs = 300;
  static constexpr unsigned long BoardMotionMinimumMs = 150;
};
