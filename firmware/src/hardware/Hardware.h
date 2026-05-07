#pragma once

#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_VL53L0X.h>
#include <U8g2lib.h>

enum class AppMode;

struct SensorFrame
{
  bool tofValid = false;
  uint16_t tofMm = 0;
  bool imuValid = false;
  sensors_event_t accel{};
  sensors_event_t gyro{};
  sensors_event_t temp{};
};

class Hardware
{
public:
  void beginModeSwitch();
  AppMode readModeSwitch() const;
  void beginBase();
  void beginSensors(bool verbose);
  void beginStartupSequence();
  void pollSerialControl();
  bool isStartupSequenceActive(unsigned long now) const;
  bool shouldShowHandshakeAnimation() const;
  void renderStartupSequence(unsigned long now);
  void renderHandshakeAnimation(unsigned long now);
  SensorFrame readSensors();
  void renderDemo(const SensorFrame &current, const char *eventLabel);
  void renderCaptureMode();
  void updateUserLed(AppMode mode);

private:
  bool i2cResponds(uint8_t address);
  bool detectOledAddress(uint8_t &address);
  bool initDisplay(uint8_t address);
  bool initTof();
  bool initMpu();
  bool readTof(uint16_t &distanceMm);
  bool readMpu(sensors_event_t &accel, sensors_event_t &gyro, sensors_event_t &temp);
  void handleHandshakeToken(const char *line);
  void emitHandshake(const char *kind);
  void setUserLed(bool on);
  void printAddress(uint8_t address);
  void printDeviceStatus(const char *name, bool online);

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C display{U8G2_R0, U8X8_PIN_NONE};
  Adafruit_VL53L0X tof;
  Adafruit_MPU6050 mpu;
  bool displayReady = false;
  bool tofReady = false;
  bool mpuReady = false;
  bool systemHealthy = false;
  bool startupSequenceEnabled = false;
  bool startupSequenceFinished = false;
  bool startupSequenceReadyVisible = false;
  bool startupHandshakeSeen = false;
  bool startupHandshakeAnimationActive = false;
  unsigned long startupSequenceStartMs = 0;
  unsigned long startupSequenceReadyAtMs = 0;
  unsigned long startupSequenceBlankUntilMs = 0;
  unsigned long startupSequenceBlinkAtMs = 0;
  unsigned long startupLastHandshakeMs = 0;
  unsigned long startupHandshakeAnimationUntilMs = 0;
  bool startupSequenceBlinkOn = false;
  bool startupHandshakeBlinkOn = false;
  bool ledState = false;
  unsigned long lastLedToggleMs = 0;
};
