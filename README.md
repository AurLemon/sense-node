# sense-node

[中文](./README_zh.md)

Sense Node is a multimodal desktop interaction node built around a Seeed XIAO ESP32-S3, a compact sensor stack, and a lightweight desktop application.

This repository is my associate degree graduation project. Therefore, Issues are closed, and Pull Requests are not accepted.. For more information about my academic background, see my [personal site](https://aurlemon.top/).

## Overview

Sense Node explores how a compact embedded device can turn physical desk interaction into software-visible events. Instead of relying on a single sensor, the project combines distance sensing, motion sensing, and local visual feedback into one hardware node, then connects that node to a desktop-side application.

The current prototype is split into two parts:

- `firmware`: PlatformIO firmware for the ESP32-S3 device
- `app`: Electron + TypeScript + Vite desktop application

## Layouts

The repository layout is intentionally simple:

- `app/`
  Electron desktop-side project
- `app/src/main.ts`
  Electron main process entry
- `app/src/preload.ts`
  Preload bridge entry
- `app/src/renderer.ts`
  Renderer entry
- `firmware/`
  Embedded project for the hardware node
- `firmware/src/main.cpp`
  Current firmware entry and sensor polling logic
- `firmware/platformio.ini`
  PlatformIO board, framework, and library configuration

## Devices

The current firmware targets a Seeed Studio XIAO ESP32-S3 and uses the following sensor/display stack over I2C:

- `VL53L0X`
  Time-of-Flight distance sensor for proximity sensing
- `MPU6050`
  IMU for motion, vibration, and tap-related observation
- `SSD1306 OLED`
  Small display driven through `U8g2` for local state and sensor feedback

The firmware is currently built with:

- `PlatformIO`
- `Arduino framework`

## Software

The desktop side is intentionally lightweight and currently uses:

- `Electron`
- `TypeScript`
- `Vite`

Its role is to act as the host-side application layer for visualization, interaction mapping, and future desktop integration.

## License

[MIT License](./LICENSE)
