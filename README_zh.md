# sense-node

[English](./README.md)

Sense Node 是一个多模态桌面交互节点，围绕 Seeed XIAO ESP32-S3、一套紧凑的传感器组合，以及一个轻量级桌面应用构建。

这个仓库是我的专科毕设项目，故不开放 Issues、不接受 Pull Requests。关于我个人更多的学业信息，可参见我的[个人网站](https://aurlemon.top/)。

## 概述

Sense Node 关注的是如何把桌面上的物理交互，转成软件侧可感知、可响应的事件。它并不依赖单一传感器，而是把距离感知、运动感知和本地可视反馈整合到同一个硬件节点中，再把这些能力连接到桌面端应用。

当前原型分成两部分：

- `firmware`：运行在 ESP32-S3 设备上的 PlatformIO 固件
- `app`：基于 Electron + TypeScript + Vite 的桌面应用

## 结构

```txt
root
├── app/       # 桌面端应用
├── docs/      # 项目文档
└── firmware/  # ESP32-S3 固件与嵌入式资源
```

## 设备

当前固件面向 Seeed Studio XIAO ESP32-S3，并通过 I2C 使用以下传感器与显示设备：

- `VL53L0X`
  ToF 距离传感器，用于近距感知
- `MPU6050`
  IMU，用于运动、振动和敲击相关观测
- `SSD1306 OLED`
  通过 `U8g2` 驱动的小屏幕，用于本地状态与传感器反馈显示

当前固件使用：

- `PlatformIO`
- `Arduino framework`

## 软件

桌面端目前保持轻量，使用的技术栈是：

- `Electron`
- `TypeScript`
- `Vite`

它主要承担主机侧的可视化、交互映射，以及后续桌面集成的职责。

## 协议

[MIT License](./LICENSE)
