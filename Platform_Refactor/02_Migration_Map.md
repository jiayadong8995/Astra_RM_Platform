# 02 Migration Map

本文档把当前仓库的内容映射到新平台，避免平台方案脱离现实。

## 1. 当前仓库结构

```text
.
├── Chassis/
└── Gimbal/
```

其中：

- `Chassis/` 是 STM32H7 底盘控制固件
- `Gimbal/` 是 STM32F4 云台控制固件

## 2. 第一版迁移对象

只迁 `Chassis/` 到平台骨架。

原因：

- 当前任务边界更清晰
- 已有重构痕迹
- 已经在向 message bus / app-layer 收敛

## 3. 目录映射

### 3.1 generated

当前来源：

- `Chassis/Core/`
- `Chassis/Drivers/`
- `Chassis/Middlewares/`

新位置建议：

```text
runtime/generated/stm32h7_ctrl_board/
  Core/
  Drivers/
  Middlewares/
```

说明：

- `main.c`
- `freertos.c`
- `fdcan.c`
- `gpio.c`
- `spi.c`
- `usart.c`
- `stm32h7xx_it.c`

这些都先按 generated 处理，后续只保留 user hook。

### 3.2 bsp

当前来源：

- `Chassis/User/Bsp/`

新位置建议：

```text
runtime/bsp/boards/stm32h7_ctrl_board/
```

第一批设备：

- `bsp_dwt`
- `bsp_uart`
- `can_bsp`
- `bsp_PWM`

第二批设备：

- BMI088 板级适配
- 遥控器串口输入
- 电机总线收发

### 3.3 module

当前来源：

- `Chassis/User/Algorithm/`
- `Chassis/User/Controller/`
- `Chassis/User/modules/`

新位置建议：

```text
runtime/module/
  algorithm/
  controller/
  message_bus/
  safety/
  estimator/
```

迁移建议：

- `message_center` 先作为 `message_bus` 原型
- `kalman` / `mahony` / `PID` / `VMC` 等作为 module 迁移候选

### 3.4 app

当前来源：

- `Chassis/User/APP/`

新位置建议：

```text
runtime/app/balance_chassis/
  app_main/
  app_tasks/
  app_modes/
```

第一批 app 迁移对象：

- `INS_task`
- `remote_task`
- `observe_task`
- `chassis_task`
- `motor_control_task`
- `robot_def.h`

说明：

这里不是把原文件原封不动复制过去，而是：

1. 保留行为
2. 减少直接全局依赖
3. 将 task 变成 app 编排入口

## 4. 当前关键依赖问题

迁移过程中要优先处理以下问题：

### 4.1 全局变量数据流

当前存在大量：

- `extern INS`
- `extern chassis_move`
- `extern right`
- `extern left`

平台化后要逐步收敛到显式 I/O。

### 4.2 三套通信机制并存

当前存在：

1. 全局变量
2. `Chassis/User/modules/message_center`
3. 历史上的 `ARF topic` 试验方案

第一版原则：

- 不在同一控制链上继续混用三套机制
- 先定一套，再渐进替换

### 4.3 任务与业务强耦合

例如现有 task 里同时做：

- 采集
- 估计
- 控制
- 输出
- 状态机

平台化后要变成：

- BSP 提供硬件读写
- Module 提供 step
- App 负责调度和装配

## 5. 推荐的第一批迁移顺序

### Batch 1

只迁构建和目录，不改行为：

- generated
- CMake
- GCC
- flash/debug

### Batch 2

抽 BSP：

- time
- uart
- can
- imu bus

### Batch 3

抽 module：

- pid
- kalman
- vmc
- balance core

### Batch 4

重构 app：

- task 编排
- mode 管理
- 消息流重构

### Batch 5

接 replay/sim：

- log adapter
- report
- scenario
