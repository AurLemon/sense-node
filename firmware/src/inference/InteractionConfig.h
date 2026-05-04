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

  static constexpr float TapJerkThreshold = 0.8f;
  static constexpr float TapDeltaThreshold = 1.2f;
  static constexpr float TapGyroMax = 1.5f;
  static constexpr unsigned long TapPulseMs = 200;
  static constexpr unsigned long TapCooldownMs = 450;

  static constexpr float MotionDeltaThreshold = 1.5f;
  static constexpr float MotionJerkThreshold = 0.6f;
  static constexpr float MotionGyroThreshold = 1.0f;
  static constexpr unsigned long MotionSettleMs = 800;
  static constexpr unsigned long MaxMotionHoldMs = 2200;

  static constexpr float StableDeltaThreshold = 0.45f;
  static constexpr float StableJerkThreshold = 0.25f;
  static constexpr float StableGyroThreshold = 0.25f;

  static constexpr float MinimumConfidence = 0.6f;
  static constexpr unsigned long BoardMotionHoldMs = 300;
  static constexpr unsigned long BoardMotionMinimumMs = 150;
};
