# 固件事件系统说明

## 0. 总体结构

| 层级             | 输入                             | 输出                                       |
| ---------------- | -------------------------------- | ------------------------------------------ |
| IMU 模型原始输出 | IMU 六轴窗口数据                 | `idle` / `tap` / `board_motion` / `unknown` |
| IMU 后处理事件   | 模型原始输出 + 运行时特征        | `idle` / `tap` / `board_motion` / `reject` |
| ToF 手部状态     | ToF 距离                         | `no_hand` / `hand_hover` / `hand_near`     |
| ToF 一次性事件   | ToF 状态转移                     | `hand_leave`                               |
| 融合状态机       | IMU 后处理事件 + ToF 状态 / 事件 | `final_event` + `fusion_state`            |

## 1. IMU 模型原始输出

| 事件名         | 含义         | 说明         |
| -------------- | ------------ | ------------ |
| `idle`         | 板体静止候选 | 模型直接输出 |
| `tap`          | 敲击候选     | 模型直接输出 |
| `board_motion` | 板体移动候选 | 模型直接输出 |

说明：

- 原始输出仅包含以上三类
- `reject` 不属于模型原始类别

输入数据：

| 输入项       | 说明       |
| ------------ | ---------- |
| `ax, ay, az` | 加速度三轴 |
| `gx, gy, gz` | 角速度三轴 |

窗口信息：

| 项目       | 值     |
| ---------- | ------ |
| 采样间隔   | `50ms` |
| 每帧轴数   | `6`    |
| 原始样本数 | `60`   |

## 2. IMU 后处理事件

| 事件名         | 含义         | 条件                                          |
| -------------- | ------------ | --------------------------------------------- |
| `idle`         | IMU 静止事件 | 原始输出为 `idle`，且通过置信度门限           |
| `tap`          | IMU 敲击事件 | 原始输出为 `tap`，且通过冲击门限与冷却约束    |
| `board_motion` | IMU 移动事件 | 原始输出为 `board_motion`，且持续时间满足条件 |
| `reject`       | IMU 拒识事件 | 置信度不足，或后处理条件不满足                |

`tap` 当前使用的判定项：

| 字段             | 用途         |
| ---------------- | ------------ |
| `imu_label`      | 模型候选标签 |
| `imu_confidence` | 置信度门限   |
| `peak_acc_delta` | 冲击强度门限 |
| `lastTapMs`      | 冷却控制     |

运行时特征：

| 字段                | 计算或含义               |
| ------------------- | ------------------------ |
| `acc_mag`           | 当前合加速度             |
| `acc_delta`         | 合加速度与重力基线的偏离 |
| `gyro_mag`          | 当前角速度强度           |
| `motion_energy`     | 组合运动强度             |
| `peak_acc_delta`    | 当前窗口内最大冲击偏移   |
| `event_duration_ms` | 活跃状态持续时间         |

判定流程：

| 步骤 | 规则                                           |
| ---- | ---------------------------------------------- |
| 1    | 模型输出最高置信类别                           |
| 2    | 若最高置信度低于门限，输出 `reject`            |
| 3    | 若类别为 `tap`，额外检查冲击门限与冷却期       |
| 4    | 若类别为 `board_motion`，额外检查持续时间      |
| 5    | 条件满足时输出对应 IMU 事件，否则输出 `reject` |

当前实现状态：

| 项目                  | 状态   |
| --------------------- | ------ |
| `confidence` 门限     | 已实现 |
| `peak_acc_delta` 门限 | 已实现 |
| `cooldown`            | 已实现 |
| `event_duration_ms`   | 已实现 |
| `jerk` 判定           | 已实现 |
| `tap pulse`           | 已实现 |
| `board_motion settle` | 已实现 |

## 3. ToF 手部状态

| 状态名       | 含义           | 条件                                     |
| ------------ | -------------- | ---------------------------------------- |
| `no_hand`    | 无稳定手部目标 | 距离无效、超出范围，或离开后恢复为空状态 |
| `hand_hover` | 手悬停         | 距离稳定落在 hover 区间                  |
| `hand_near`  | 手靠近         | 距离稳定小于 near 阈值                   |

说明：

- `hand_state` 仅包含以上三个值

阈值项：

| 字段              | 作用                         |
| ----------------- | ---------------------------- |
| `NEAR_ENTER`      | 进入 `hand_near` 的距离门限  |
| `NEAR_EXIT`       | 退出 `hand_near` 的距离门限  |
| `HOVER_MIN`       | `hand_hover` 最小距离        |
| `HOVER_MAX_ENTER` | 进入 `hand_hover` 的最大距离 |
| `HOVER_MAX_EXIT`  | 退出 `hand_hover` 的最大距离 |
| `TOF_INVALID`     | 无效距离值                   |

状态转移规则：

| 当前状态     | 条件                                   | 下一状态     |
| ------------ | -------------------------------------- | ------------ |
| `no_hand`    | 距离稳定小于 `NEAR_ENTER`              | `hand_near`  |
| `no_hand`    | 距离稳定落在 hover 区间                | `hand_hover` |
| `hand_near`  | 距离大于 `NEAR_EXIT` 且仍在 hover 区间 | `hand_hover` |
| `hand_near`  | 距离无效或大于 `HOVER_MAX_EXIT`        | `no_hand`    |
| `hand_hover` | 距离小于 `NEAR_ENTER`                  | `hand_near`  |
| `hand_hover` | 距离无效或大于 `HOVER_MAX_EXIT`        | `no_hand`    |

稳定时间：

| 场景                            | 约束                     |
| ------------------------------- | ------------------------ |
| 进入 `hand_hover` / `hand_near` | 需要持续超过稳定时间门限 |
| 回到 `no_hand`                  | 需要持续超过离开时间门限 |

## 4. ToF 一次性事件

| 事件名       | 含义   | 条件                                                              |
| ------------ | ------ | ----------------------------------------------------------------- |
| `hand_leave` | 手离开 | 从 `hand_hover` 或 `hand_near` 回到 `no_hand`，并持续超过离开门限 |

说明：

- `hand_leave` 不属于 `hand_state`
- `hand_leave` 为一次性事件

触发过程：

| 步骤 | 条件                                       |
| ---- | ------------------------------------------ |
| 1    | 上一稳定状态为 `hand_hover` 或 `hand_near` |
| 2    | 当前候选状态变为 `no_hand`                 |
| 3    | `no_hand` 持续超过离开时间门限             |
| 4    | 触发一次 `hand_leave`                      |

## 5. 融合状态机最终事件

| 事件名         | 含义         | 输出方式                   |
| -------------- | ------------ | -------------------------- |
| `idle`         | 默认静止事件 | 持续输出                   |
| `tap`          | 敲击脉冲事件 | 触发一次，随后进入冷却期   |
| `board_motion` | 板体移动事件 | 持续输出，但可快速退出     |
| `reject`       | 不确定事件   | 置信度不足或状态冲突时输出 |
| `unknown`      | 初始化兜底   | 仍在 warmup 时输出         |

说明：

- `final_event` 现在只承载 IMU / 融合后的主事件语义
- `hand_state` 作为独立字段保留，不再直接覆盖 `final_event`
- ToF 仍然参与融合判断，但不会直接否定 `tap`

融合输入：

| 输入项         | 来源           |
| -------------- | -------------- |
| `motion_event` | IMU 后处理事件 |
| `hand_state`   | ToF 手部状态   |
| `hand_leave`   | ToF 一次性事件 |
| `confidence`   | IMU 最高置信度 |

融合规则：

| 情况                                     | 最终输出       |
| ---------------------------------------- | -------------- |
| `tap` 候选且未处于冷却期                 | `tap`          |
| `board_motion` 候选且满足持续性条件      | `board_motion` |
| 原始 IMU 稳定且无其他事件                | `idle`         |
| 置信度不足、冲突、或无法稳定判断         | `reject`       |
| 仍处于 warmup                           | `unknown`      |

补充规则：

| 规则                | 说明                                         |
| ------------------- | -------------------------------------------- |
| `tap` 冷却          | 冷却期内不重复触发 `tap`                     |
| `board_motion` 保持 | 触发后维持短时间输出                         |
| ToF 参与            | ToF 只参与辅助判断，不直接覆盖 IMU 主事件    |

## 6. 事件优先级

```txt
```txt
tap > board_motion > idle > reject > unknown
```
```

| 优先级 | 事件名         |
| ------ | -------------- |
| 1      | `tap`          |
| 2      | `board_motion` |
| 3      | `idle`         |
| 4      | `reject`       |
| 5      | `unknown`      |

## 7. 串口返回值

Demo 模式下，串口有两类输出：

- `warmup_info`：进入 Demo 且推理窗口未填满时输出一次
- 常态 CSV：每次 `DemoSerialTick` 输出一行

### 7.1 `warmup_info`

`warming_up` 表示 IMU 推理窗口尚未填满，当前仍在采样阶段，尚未执行有效分类。

输出示例：

```txt
warmup_info
model_labels=3
sample_interval_ms=50
window_frames=60
window_fill=8/60
warmup_target_ms=3000
warmup_remaining_ms=2600
dsp_input_frame_size=360
nn_input_frame_size=78
classifier_threshold=0.6000
cpu_mhz=240
free_heap=278000
min_free_heap=275000
max_alloc_heap=110000
```

字段说明：

| 字段名                 | 含义                                     |
| ---------------------- | ---------------------------------------- |
| `model_labels`         | 模型类别数                               |
| `sample_interval_ms`   | 采样间隔，单位 `ms`                      |
| `window_frames`        | 推理窗口总帧数                           |
| `window_fill`          | 当前已采样帧数 / 总帧数                  |
| `warmup_target_ms`     | 填满一个推理窗口所需总时长，单位 `ms`    |
| `warmup_remaining_ms`  | 距离窗口填满剩余时长，单位 `ms`          |
| `dsp_input_frame_size` | DSP 输入维度总长度                       |
| `nn_input_frame_size`  | 神经网络输入维度长度                     |
| `classifier_threshold` | 当前置信度门限                           |
| `cpu_mhz`              | ESP32-S3 当前 CPU 频率，单位 `MHz`       |
| `free_heap`            | 当前可用堆内存，单位 `bytes`             |
| `min_free_heap`        | 运行至今最小可用堆内存，单位 `bytes`     |
| `max_alloc_heap`       | 当前可一次性分配的最大堆块，单位 `bytes` |

### 7.2 常态 CSV

当前输出顺序：

```txt
tof, ax, ay, az, gx, gy, gz, uptime, imu_label, imu_confidence, hand_state, motion_event, final_event, dsp_ms, classification_ms, postprocessing_ms, window_fill, cpu_mhz, free_heap, min_free_heap, max_alloc_heap
```

字段说明：

| 序号 | 字段名              | 含义                                                       |
| ---- | ------------------- | ---------------------------------------------------------- |
| 1    | `tof`               | ToF 距离，单位 `mm`。无效时输出 `-1`                       |
| 2    | `ax`                | X 轴加速度                                                 |
| 3    | `ay`                | Y 轴加速度                                                 |
| 4    | `az`                | Z 轴加速度                                                 |
| 5    | `gx`                | X 轴角速度                                                 |
| 6    | `gy`                | Y 轴角速度                                                 |
| 7    | `gz`                | Z 轴角速度                                                 |
| 8    | `uptime`            | 设备运行时间，格式为 `Xs`                                  |
| 9    | `imu_label`         | IMU 模型原始分类结果。窗口未满时输出 `warming_up`          |
| 10   | `imu_confidence`    | IMU 模型最高置信度                                         |
| 11   | `hand_state`        | ToF 当前状态：`no_hand` / `hand_hover` / `hand_near`       |
| 12   | `motion_event`      | IMU 后处理事件：`idle` / `tap` / `board_motion` / `reject` |
| 13   | `final_event`       | 融合状态机最终输出。窗口未满时输出 `warming_up`            |
| 14   | `dsp_ms`            | 当前一次推理的 DSP 耗时，单位 `ms`                         |
| 15   | `classification_ms` | 当前一次推理的分类耗时，单位 `ms`                          |
| 16   | `postprocessing_ms` | 当前一次推理的后处理耗时，单位 `ms`                        |
| 17   | `window_fill`       | 当前窗口填充进度，格式为 `已采样帧数/总帧数`               |
| 18   | `cpu_mhz`           | ESP32-S3 当前 CPU 频率，单位 `MHz`                         |
| 19   | `free_heap`         | 当前可用堆内存，单位 `bytes`                               |
| 20   | `min_free_heap`     | 运行至今最小可用堆内存，单位 `bytes`                       |
| 21   | `max_alloc_heap`    | 当前可一次性分配的最大堆块，单位 `bytes`                   |

取值补充：

| 字段名         | 说明                                              |
| -------------- | ------------------------------------------------- |
| `imu_label`    | 来自 Edge Impulse 原始分类结果                    |
| `motion_event` | 来自 IMU 后处理结果                               |
| `hand_state`   | 来自 ToF 状态机                                   |
| `final_event`  | 来自融合状态机                                    |
| `window_fill`  | 推理窗口填充进度，填满后才能产生稳定 IMU 推理结果 |

## 8. 后续接入建议字段

| 字段名          | 用途           |
| --------------- | -------------- |
| `distance_mm`   | ToF 原始距离   |
| `motion_event`  | IMU 后处理事件 |
| `hand_event`    | ToF 一次性事件 |
| `acc_delta`     | 运动阈值调参   |
| `reject_reason` | 拒识原因记录   |

`reject_reason` 建议值域：

| 值                | 含义           |
| ----------------- | -------------- |
| `low_confidence`  | 置信度不足     |
| `impact_too_weak` | 冲击不足       |
| `cooldown`        | 冷却期内       |
| `tof_unstable`    | ToF 状态不稳定 |

## 9. OLED 输出

| 字段名        | 含义         |
| ------------- | ------------ |
| `final_event` | 当前最终事件 |

## 10. 表情映射

| 最终事件 | 表情 | 说明 |
| -------- | ---- | ---- |
| `idle` | `:)` | 默认静止态 |
| `tap` | `o_o` | 点击瞬态 |
| `board_motion` | `>_<` | 板体运动态 |

说明：

- `reject` 不显示
- `unknown` 不显示
- 该映射用于 firmware OLED 与 Electron 主页 scene 的统一展示
