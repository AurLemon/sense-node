#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_VL53L0X.h>
#include <U8g2lib.h>

namespace
{
  constexpr uint8_t kSdaPin = 5;
  constexpr uint8_t kSclPin = 6;
  constexpr uint32_t kSerialBaudRate = 115200;
  constexpr unsigned long kDisplayIntervalMs = 33;
  constexpr unsigned long kSerialIntervalMs = 1000;
  constexpr unsigned long kLedBlinkIntervalMs = 500;

  constexpr uint8_t kOledAddressPrimary = 0x3C;
  constexpr uint8_t kOledAddressSecondary = 0x3D;
  constexpr uint8_t kTofAddress = 0x29;
  constexpr uint8_t kMpuAddressPrimary = 0x68;
  constexpr uint8_t kMpuAddressSecondary = 0x69;

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
  Adafruit_VL53L0X tof;
  Adafruit_MPU6050 mpu;

  struct SensorFrame
  {
    bool tofValid = false;
    uint16_t tofMm = 0;
    bool imuValid = false;
    sensors_event_t accel{};
    sensors_event_t gyro{};
    sensors_event_t temp{};
  };

  bool gDisplayReady = false;
  bool gTofReady = false;
  bool gMpuReady = false;
  bool gSystemHealthy = false;
  bool gLedState = false;
  unsigned long gLastLedToggleMs = 0;
  unsigned long gLastDisplayMs = 0;
  unsigned long gLastSerialMs = 0;
  SensorFrame gLatestFrame{};
  SensorFrame gPreviousFrame{};

  bool i2cResponds(uint8_t address)
  {
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
  }

  void printAddress(uint8_t address)
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

  void printDeviceStatus(const char *name, bool online)
  {
    Serial.print(name);
    Serial.print(": ");
    Serial.println(online ? "online" : "offline");
  }

  void setUserLed(bool on)
  {
#ifdef LED_BUILTIN
    digitalWrite(LED_BUILTIN, on ? LOW : HIGH);
#else
    (void)on;
#endif
  }

  void updateUserLed()
  {
    if (gSystemHealthy)
    {
      setUserLed(true);
      return;
    }

    const unsigned long now = millis();
    if (now - gLastLedToggleMs >= kLedBlinkIntervalMs)
    {
      gLastLedToggleMs = now;
      gLedState = !gLedState;
      setUserLed(gLedState);
    }
  }

  bool detectOledAddress(uint8_t &address)
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

  bool initDisplay(uint8_t address)
  {
    display.setI2CAddress(static_cast<uint8_t>(address << 1));
    display.begin();
    display.setFont(u8g2_font_6x10_tr);
    display.setFontPosTop();
    return true;
  }

  bool initTof()
  {
    return tof.begin();
  }

  bool initMpu()
  {
    return mpu.begin();
  }

  bool readTof(uint16_t &distanceMm)
  {
    VL53L0X_RangingMeasurementData_t measure;
    tof.rangingTest(&measure, false);

    if (measure.RangeStatus == 4)
    {
      return false;
    }

    distanceMm = measure.RangeMilliMeter;
    return true;
  }

  bool readMpu(sensors_event_t &accel, sensors_event_t &gyro, sensors_event_t &temp)
  {
    mpu.getEvent(&accel, &gyro, &temp);
    return true;
  }

  float absf(float value)
  {
    return value < 0.0f ? -value : value;
  }

  float squared(float value)
  {
    return value * value;
  }

  void copyFrame(SensorFrame &dst, const SensorFrame &src)
  {
    dst = src;
  }

  const char *getImuMotionLabel(const SensorFrame &current, const SensorFrame &previous)
  {
    if (!current.imuValid || !previous.imuValid)
    {
      return "---";
    }

    const float accelDelta = sqrtf(
        squared(current.accel.acceleration.x - previous.accel.acceleration.x) +
        squared(current.accel.acceleration.y - previous.accel.acceleration.y) +
        squared(current.accel.acceleration.z - previous.accel.acceleration.z));

    const float gyroDelta = sqrtf(
        squared(current.gyro.gyro.x - previous.gyro.gyro.x) +
        squared(current.gyro.gyro.y - previous.gyro.gyro.y) +
        squared(current.gyro.gyro.z - previous.gyro.gyro.z));

    const float motionScore = accelDelta + gyroDelta * 0.35f;

    if (motionScore < 0.12f)
    {
      return "STIL";
    }

    if (motionScore < 0.45f)
    {
      return "LOW";
    }

    if (motionScore < 1.2f)
    {
      return "MOVE";
    }

    return "JOLT";
  }

  void renderDisplay(const SensorFrame &frame)
  {
    char line[32];
    const char *systemLabel = gSystemHealthy ? "[OK]" : "[ERR]";
    const char *motionLabel = getImuMotionLabel(frame, gPreviousFrame);

    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tr);
    display.drawStr(0, 10, "SenseNode");
    display.drawStr(68, 10, systemLabel);
    display.drawStr(98, 10, motionLabel);

    if (frame.tofValid)
    {
      snprintf(line, sizeof(line), "TOF: %u mm", frame.tofMm);
    }
    else
    {
      snprintf(line, sizeof(line), "TOF: --");
    }
    display.drawStr(0, 24, line);

    if (frame.imuValid)
    {
      snprintf(line, sizeof(line), "A: %+.1f %+.1f %+.1f", frame.accel.acceleration.x,
               frame.accel.acceleration.y, frame.accel.acceleration.z);
    }
    else
    {
      snprintf(line, sizeof(line), "A: --");
    }
    display.drawStr(0, 38, line);

    if (frame.imuValid)
    {
      snprintf(line, sizeof(line), "G: %+.1f %+.1f %+.1f", frame.gyro.gyro.x, frame.gyro.gyro.y,
               frame.gyro.gyro.z);
    }
    else
    {
      snprintf(line, sizeof(line), "G: --");
    }
    display.drawStr(0, 52, line);

    display.sendBuffer();
  }

  void printFrameToSerial(const SensorFrame &frame)
  {
    Serial.println();

    if (frame.tofValid)
    {
      Serial.print("TOF: ");
      Serial.print(static_cast<float>(frame.tofMm), 5);
      Serial.println(" mm");
    }
    else
    {
      Serial.println("TOF: --");
    }

    if (frame.imuValid)
    {
      Serial.print("A: ");
      Serial.print(frame.accel.acceleration.x, 5);
      Serial.print(", ");
      Serial.print(frame.accel.acceleration.y, 5);
      Serial.print(", ");
      Serial.println(frame.accel.acceleration.z, 5);

      Serial.print("G: ");
      Serial.print(frame.gyro.gyro.x, 5);
      Serial.print(", ");
      Serial.print(frame.gyro.gyro.y, 5);
      Serial.print(", ");
      Serial.println(frame.gyro.gyro.z, 5);
    }
    else
    {
      Serial.println("IMU: --");
    }
  }
} // namespace

void setup()
{
  Serial.begin(kSerialBaudRate);
  delay(1000);

#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  setUserLed(false);
#endif

  Wire.begin(kSdaPin, kSclPin);
  Wire.setClock(100000);

  Serial.println();
  Serial.println("XIAO ESP32S3 I2C sensor monitor");
  Serial.print("SDA = GPIO");
  Serial.println(kSdaPin);
  Serial.print("SCL = GPIO");
  Serial.println(kSclPin);

  const bool tofPresent = i2cResponds(kTofAddress);
  const bool mpuPresent68 = i2cResponds(kMpuAddressPrimary);
  const bool mpuPresent69 = i2cResponds(kMpuAddressSecondary);
  uint8_t oledAddress = 0;
  const bool oledPresent = detectOledAddress(oledAddress);
  const bool mpuOnline = mpuPresent68 || mpuPresent69;

  Serial.println("Startup I2C check:");
  printDeviceStatus("OLED (0x3C/0x3D)", oledPresent);
  printDeviceStatus("VL53L0X (0x29)", tofPresent);
  printDeviceStatus("MPU6050 (0x68/0x69)", mpuOnline);

  gSystemHealthy = oledPresent && tofPresent && mpuOnline;

  if (oledPresent)
  {
    Serial.print("OLED address: ");
    printAddress(oledAddress);
    Serial.println();
    gDisplayReady = initDisplay(oledAddress);
    if (!gDisplayReady)
    {
      Serial.println("OLED init failed, keeping serial output.");
    }
  }
  else
  {
    Serial.println("OLED not found, skipping display init.");
  }

  if (tofPresent)
  {
    gTofReady = initTof();
    if (!gTofReady)
    {
      Serial.println("VL53L0X init failed.");
    }
  }
  else
  {
    Serial.println("VL53L0X not found, skipping ranging init.");
  }

  if (mpuOnline)
  {
    if (mpuPresent69 && !mpuPresent68)
    {
      Serial.println("MPU6050 detected at 0x69. This sketch initializes the default address.");
      Serial.println("If init fails, connect AD0 to GND for 0x68.");
    }
    gMpuReady = initMpu();
    if (!gMpuReady)
    {
      Serial.println("MPU6050 init failed.");
    }
  }
  else
  {
    Serial.println("MPU6050 not found, skipping IMU init.");
  }

  gLastDisplayMs = millis();
  gLastSerialMs = millis();
  gLatestFrame.tofValid = gTofReady && readTof(gLatestFrame.tofMm);
  gLatestFrame.imuValid = gMpuReady && readMpu(gLatestFrame.accel, gLatestFrame.gyro, gLatestFrame.temp);
  if (gDisplayReady)
  {
    renderDisplay(gLatestFrame);
  }
  printFrameToSerial(gLatestFrame);

  if (gSystemHealthy)
  {
    Serial.println("User LED: steady on");
    setUserLed(true);
  }
  else
  {
    Serial.println("User LED: blinking");
    gLastLedToggleMs = millis();
    gLedState = false;
    setUserLed(false);
  }
}

void loop()
{
  updateUserLed();

  const unsigned long now = millis();

  if (now - gLastDisplayMs >= kDisplayIntervalMs)
  {
    gLastDisplayMs = now;
    copyFrame(gPreviousFrame, gLatestFrame);
    gLatestFrame.tofValid = gTofReady && readTof(gLatestFrame.tofMm);
    gLatestFrame.imuValid = gMpuReady && readMpu(gLatestFrame.accel, gLatestFrame.gyro, gLatestFrame.temp);

    if (gDisplayReady)
    {
      renderDisplay(gLatestFrame);
    }
  }

  if (now - gLastSerialMs >= kSerialIntervalMs)
  {
    gLastSerialMs = now;
    printFrameToSerial(gLatestFrame);
  }

  delay(1);
}
