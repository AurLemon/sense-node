#include "hardware/Hardware.h"

#include "events/EventBus.h"

namespace
{
  constexpr uint8_t kSdaPin = 5;
  constexpr uint8_t kSclPin = 6;
  constexpr uint8_t kModeSwitchPin = 1;
  constexpr unsigned long kErrorBlinkIntervalMs = 500;
  constexpr unsigned long kCaptureBlinkIntervalMs = 1500;

  constexpr uint8_t kOledAddressPrimary = 0x3C;
  constexpr uint8_t kOledAddressSecondary = 0x3D;
  constexpr uint8_t kTofAddress = 0x29;
  constexpr uint16_t kTofOutOfRangeMm = 8191;
  constexpr uint8_t kMpuAddressPrimary = 0x68;
  constexpr uint8_t kMpuAddressSecondary = 0x69;
}

void Hardware::beginModeSwitch()
{
  pinMode(kModeSwitchPin, INPUT_PULLUP);
}

AppMode Hardware::readModeSwitch() const
{
  return digitalRead(kModeSwitchPin) == LOW ? AppMode::Capture : AppMode::Demo;
}

void Hardware::beginBase()
{
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  setUserLed(false);
#endif

  Wire.begin(kSdaPin, kSclPin);
  Wire.setClock(100000);
}

void Hardware::beginSensors(bool verbose)
{
  if (verbose)
  {
    Serial.println();
    Serial.println("XIAO ESP32S3 I2C sensor monitor");
    Serial.print("SDA = GPIO");
    Serial.println(kSdaPin);
    Serial.print("SCL = GPIO");
    Serial.println(kSclPin);
  }

  const bool tofPresent = i2cResponds(kTofAddress);
  const bool mpuPresent68 = i2cResponds(kMpuAddressPrimary);
  const bool mpuPresent69 = i2cResponds(kMpuAddressSecondary);
  uint8_t oledAddress = 0;
  const bool oledPresent = detectOledAddress(oledAddress);
  const bool mpuOnline = mpuPresent68 || mpuPresent69;

  if (verbose)
  {
    Serial.println("Startup I2C check:");
    printDeviceStatus("OLED (0x3C/0x3D)", oledPresent);
    printDeviceStatus("VL53L0X (0x29)", tofPresent);
    printDeviceStatus("MPU6050 (0x68/0x69)", mpuOnline);
  }

  systemHealthy = oledPresent && tofPresent && mpuOnline;

  if (oledPresent)
  {
    if (verbose)
    {
      Serial.print("OLED address: ");
      printAddress(oledAddress);
      Serial.println();
    }
    displayReady = initDisplay(oledAddress);
    if (verbose && !displayReady)
    {
      Serial.println("OLED init failed, keeping serial output.");
    }
  }
  else if (verbose)
  {
    Serial.println("OLED not found, skipping display init.");
  }

  if (tofPresent)
  {
    tofReady = initTof();
    if (verbose && !tofReady)
    {
      Serial.println("VL53L0X init failed.");
    }
  }
  else if (verbose)
  {
    Serial.println("VL53L0X not found, skipping ranging init.");
  }

  if (mpuOnline)
  {
    if (verbose && mpuPresent69 && !mpuPresent68)
    {
      Serial.println("MPU6050 detected at 0x69. This sketch initializes the default address.");
      Serial.println("If init fails, connect AD0 to GND for 0x68.");
    }
    mpuReady = initMpu();
    if (verbose && !mpuReady)
    {
      Serial.println("MPU6050 init failed.");
    }
  }
  else if (verbose)
  {
    Serial.println("MPU6050 not found, skipping IMU init.");
  }
}

SensorFrame Hardware::readSensors()
{
  SensorFrame frame{};
  frame.tofValid = tofReady && readTof(frame.tofMm);
  frame.imuValid = mpuReady && readMpu(frame.accel, frame.gyro, frame.temp);
  return frame;
}

void Hardware::renderDemo(const SensorFrame &current, const char *eventLabel)
{
  if (!displayReady)
  {
    return;
  }

  char topLine[16];
  char line[32];
  const char *systemLabel = systemHealthy ? "[OK]" : "[ERR]";

  if (current.tofValid && current.tofMm < kTofOutOfRangeMm)
  {
    snprintf(topLine, sizeof(topLine), "%u mm", current.tofMm);
  }
  else
  {
    snprintf(topLine, sizeof(topLine), "-");
  }

  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tr);
  display.drawStr(0, 10, "SenseNode");
  const uint8_t statusWidth = display.getStrWidth(systemLabel);
  const uint8_t tofWidth = display.getStrWidth(topLine);
  const uint8_t statusX = 128 - statusWidth;
  const uint8_t tofX = statusX > (tofWidth + 3) ? statusX - tofWidth - 3 : 0;
  display.drawStr(tofX, 10, topLine);
  display.drawStr(statusX, 10, systemLabel);
  display.drawStr(0, 24, eventLabel);

  if (current.imuValid)
  {
    snprintf(line, sizeof(line), "A: %+.1f %+.1f %+.1f", current.accel.acceleration.x,
             current.accel.acceleration.y, current.accel.acceleration.z);
  }
  else
  {
    snprintf(line, sizeof(line), "A: --");
  }
  display.drawStr(0, 38, line);

  if (current.imuValid)
  {
    snprintf(line, sizeof(line), "G: %+.1f %+.1f %+.1f", current.gyro.gyro.x, current.gyro.gyro.y,
             current.gyro.gyro.z);
  }
  else
  {
    snprintf(line, sizeof(line), "G: --");
  }
  display.drawStr(0, 52, line);

  display.sendBuffer();
}

void Hardware::renderCaptureMode()
{
  if (!displayReady)
  {
    return;
  }

  display.clearBuffer();
  display.setFont(u8g2_font_7x14B_tr);
  display.setFontPosTop();
  display.drawStr(0, 24, "Capture Mode");
  display.sendBuffer();
}

void Hardware::updateUserLed(AppMode mode)
{
  if (mode == AppMode::Capture)
  {
    const unsigned long now = millis();
    if (now - lastLedToggleMs >= kCaptureBlinkIntervalMs)
    {
      lastLedToggleMs = now;
      ledState = !ledState;
      setUserLed(ledState);
    }
    return;
  }

  if (systemHealthy)
  {
    setUserLed(true);
    return;
  }

  const unsigned long now = millis();
  if (now - lastLedToggleMs >= kErrorBlinkIntervalMs)
  {
    lastLedToggleMs = now;
    ledState = !ledState;
    setUserLed(ledState);
  }
}

bool Hardware::i2cResponds(uint8_t address)
{
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool Hardware::detectOledAddress(uint8_t &address)
{
  if (i2cResponds(kOledAddressPrimary))
  {
    address = kOledAddressPrimary;
    return true;
  }

  if (i2cResponds(kOledAddressSecondary))
  {
    address = kOledAddressSecondary;
    return true;
  }

  return false;
}

bool Hardware::initDisplay(uint8_t address)
{
  display.setI2CAddress(static_cast<uint8_t>(address << 1));
  display.begin();
  display.setFont(u8g2_font_6x10_tr);
  display.setFontPosTop();
  return true;
}

bool Hardware::initTof()
{
  return tof.begin();
}

bool Hardware::initMpu()
{
  return mpu.begin();
}

bool Hardware::readTof(uint16_t &distanceMm)
{
  VL53L0X_RangingMeasurementData_t measure;
  tof.rangingTest(&measure, false);

  if (measure.RangeStatus == 4)
  {
    distanceMm = kTofOutOfRangeMm;
    return true;
  }

  distanceMm = measure.RangeMilliMeter;
  return true;
}

bool Hardware::readMpu(sensors_event_t &accel, sensors_event_t &gyro, sensors_event_t &temp)
{
  mpu.getEvent(&accel, &gyro, &temp);
  return true;
}

void Hardware::setUserLed(bool on)
{
#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, on ? LOW : HIGH);
#else
  (void)on;
#endif
}

void Hardware::printAddress(uint8_t address)
{
  if (address < 0x10)
  {
    Serial.print("0x0");
  }
  else
  {
    Serial.print("0x");
  }
  Serial.print(address, HEX);
}

void Hardware::printDeviceStatus(const char *name, bool online)
{
  Serial.print(name);
  Serial.print(": ");
  Serial.println(online ? "online" : "offline");
}
