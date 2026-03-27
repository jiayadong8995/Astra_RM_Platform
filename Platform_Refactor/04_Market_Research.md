# 开源方案调研：嵌入式机器人平台 + SIL/Replay

## 最相关的 5 个开源项目

### 1. PX4 Autopilot — ⭐ 最值得参考

| 项 | 详情 |
|---|---|
| 定位 | 无人机飞控，全球最大开源飞控项目 |
| MCU | STM32F4/F7/H7, NuttX RTOS |
| 消息总线 | **uORB** — 轻量级 pub/sub，单缓冲区，lock-free |
| SIL/SITL | ✅ 整个飞控固件在 PC 上运行，通过 MAVLink 与仿真器 (Gazebo) 通信 |
| 架构核心 | 每个模块是一个独立进程/线程，通过 uORB topic 交换数据 |

**它怎么解决 PC 仿真的问题：**
- PX4 跑在 NuttX 上，NuttX 有 POSIX 仿真层 → 整个固件可以直接编译为 Linux 进程
- SITL 模式：固件在 PC 运行，仿真器喂传感器数据，固件输出电机指令
- **Lockstep**：仿真器和固件同步推进时间，保证可重复性

**对你的参考价值：**
- uORB 的 pub/sub 设计思路，你的 ARF topic 已经非常接近
- uORB 有一个 [POSIX standalone port](https://github.com/nicoring/uorb_posix)，可参考
- 但 PX4 整体太重（几十万行），不适合直接搬

---

### 2. Betaflight — ⭐ 架构参考

| 项 | 详情 |
|---|---|
| 定位 | FPV 穿越机飞控 |
| MCU | STM32F4/F7/H7, **裸机** (无 RTOS) |
| 调度 | **协作式调度器** — 按优先级/周期调度 task，每个 task 是一个 step 函数 |
| SITL | ✅ 固件编译为 PC 二进制，接外部仿真器 |

**它怎么做到裸机 SITL 的：**
```c
// Betaflight 的 task 定义 — 就是 step 函数
void taskGyro(timeUs_t currentTimeUs) { ... }   // 陀螺仪读取+滤波
void taskPid(timeUs_t currentTimeUs) { ... }     // PID 计算
void taskRx(timeUs_t currentTimeUs) { ... }      // 遥控器输入
```
- 调度器按 `desiredPeriodUs` 和优先级依次调用这些 step 函数
- SITL 时：替换底层硬件驱动为 socket/共享内存接口，调度器逻辑不变
- **关键：它没有 RTOS，所以不存在"抽象 OS"的问题**

**对你的参考价值：**
- 你用 FreeRTOS，但 task step 化的思路完全可以借鉴
- 证明了 "step 函数 + 调度器" 是可行的嵌入式 SIL 方案

---

### 3. iNav — HITL/SITL 参考

| 项 | 详情 |
|---|---|
| 定位 | 导航飞控（从 Cleanflight 分支）|
| SITL | ✅ 固件在 PC 运行，接 RealFlight 等仿真器 |
| HITL | ✅ **真实飞控板** 接仿真器（X-Plane），用虚拟传感器数据 |

**HITL 对你有意义**：你可以先不做 PC 编译，而是在**真实 STM32 板上**接一个 PC 端仿真环境喂数据。这条路不需要改代码架构。

---

### 4. HoneyGrant/STM32_RoboMaster — RM 生态

| 项 | 详情 |
|---|---|
| 定位 | RoboMaster 嵌入式框架（湖南大学岳麓战队）|
| 分层 | BSP → Module → App (三层) |
| Pub/Sub | ✅ app 层 pub/sub 解耦 |
| SIL | ❌ 无 |

**架构和你的设计非常接近**：BSP/Module/App 三层 + pub/sub。但它没有任何 SIL/replay 能力。

---

### 5. RoboRTS — RM + ROS

| 项 | 详情 |
|---|---|
| 定位 | RoboMaster AI 机器人开源平台 |
| 上位机 | ROS |
| 下位机 | STM32 裸机控制器 |
| SIL | 依赖 ROS + Gazebo |

**太重了**，需要 ROS 生态，不适合你的纯嵌入式场景。

---

## 对比总结

| | PX4 | Betaflight | iNav | HoneyGrant | RoboRTS | **你的平台** |
|---|---|---|---|---|---|---|
| MCU | STM32 + NuttX | STM32 裸机 | STM32 裸机 | STM32 + FreeRTOS | STM32 + ROS | STM32 + FreeRTOS |
| Pub/Sub | uORB ✅ | 无 | 无 | 简单 pub/sub ✅ | ROS topic | ARF topic / message_center |
| SITL | ✅ 完整 | ✅ 完整 | ✅ 完整 | ❌ | ✅ via ROS | ❌ 待建 |
| Replay | 有 log 工具 | Blackbox log ✅ | Blackbox log ✅ | ❌ | ROS bag ✅ | ❌ 待建 |
| 适合直接用？ | ❌ 太大 | ❌ 裸机，无 RTOS | ❌ 裸机 | ❌ 无 SIL | ❌ 需要 ROS | — |

## 结论

**没有现成方案可以直接拿来用**，但有非常好的设计参考：

| 你需要解决的问题 | 最佳参考 |
|---|---|
| module 层 pub/sub 设计 | PX4 uORB（你的 ARF topic 已经很接近）|
| step 函数 + 调度器模式 | Betaflight task scheduler |
| PC 端 SITL 架构 | PX4 SITL lockstep |
| 日志格式 + replay | Betaflight/iNav Blackbox log |
| 不改架构就能做的硬件仿真 | iNav HITL（板上跑固件，PC 喂数据）|

### 我的建议

**短期**（不改架构）：走 iNav HITL 路线
- 板上跑你的 legacy 固件
- PC 端 Python 通过串口/CAN 喂虚拟 IMU 数据
- 不需要改任何 C 代码

**中期**（逐步改造）：走 Betaflight step 化 + PX4 uORB 路线
- 把每个 FreeRTOS task 的核心逻辑抽成 step 函数
- 用 ARF topic 统一消息总线
- module 层可独立编译为 `.so`，PC 上跑 replay/sim
