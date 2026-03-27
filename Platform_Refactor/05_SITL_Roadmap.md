# 机器人开发平台 — 修订技术路线与实施规划

> 基于 2025-03-27 审计讨论的最终结论，替代原 `03_MVP_Roadmap.md`。

---

## 一、核心架构决策记录

### 决策 1：验证链主力 → SITL

| 被否定的方案 | 否定理由 |
|---|---|
| ~~自建 osal~~ | OS 接口太多（thread/mutex/queue），本质是造 FreeRTOS 模拟器 |
| ~~HITL（板子+模拟数据）~~ | SPI/FDCAN 无法从外部模拟，仍需改代码，不如 SITL |
| ~~step SIL 作为主力~~ | 测不到 RTOS 调度时序，2ms 控制周期的时序问题覆盖不到 |

**最终选择：SITL（Software-in-the-Loop）**
- 整个固件编译为 Linux 进程
- FreeRTOS 使用官方 POSIX/Linux 端口
- bsp 层 HAL 驱动替换为 socket 桩
- 业务代码（app + module）零修改

### 决策 2：消息总线 → ARF topic

- `ARF topic` 作为唯一主线（零动态分配、编译期静态、关中断保护）
- `message_center` 渐进替换为 ARF topic
- 全局变量逐步收敛为 topic 订阅

### 决策 3：未来上层 PC → micro-ROS

- 不自造通信协议
- 未来加上层 PC（视觉/导航）时，用 micro-ROS 让 STM32 成为 ROS 2 原生节点
- 当前阶段不做，不影响平台建设

### 决策 4：step SIL → 补充手段

- module 层逐步 step 化（纯函数）
- 用于算法单元测试，不作为主力验证链
- 改造节奏跟随消息总线收敛进度

---

## 二、SITL 架构总览

```
┌──────────────── Linux 进程 ──────────────────┐
│                                               │
│  app 层 (完全不改)                             │
│  ├── INS_task      (osPriorityRealtime, 1ms)  │
│  ├── Chassis_task  (osPriorityAboveNormal, 2ms)│
│  ├── Observe_task  (osPriorityHigh, 3ms)       │
│  ├── Motor_Control (osPriorityAboveNormal)     │
│  └── Remote_task   (osPriorityAboveNormal,10ms)│
│                                               │
│  module 层 (完全不改)                           │
│  ├── VMC  ├── LQR  ├── PID                    │
│  ├── Kalman  ├── Mahony  ├── EKF              │
│  └── message_center / ARF topic               │
│                                               │
│  bsp 层 (替换为 SITL 桩)                       │
│  ├── bmi088_sitl.c   ← socket 读虚拟 IMU      │
│  ├── fdcan_sitl.c    ← socket 收发虚拟电机帧   │
│  ├── uart_sitl.c     ← socket 读虚拟遥控器     │
│  ├── dwt_sitl.c      ← clock_gettime          │
│  ├── pwm_sitl.c      ← 空实现                  │
│  └── gpio_sitl.c     ← 空实现                  │
│                                               │
│  FreeRTOS Linux POSIX port (官方现成)           │
│  CMSIS-RTOS wrapper (现有代码直接用)            │
├───────────────────┬───────────────────────────┤
│                   │ UDP sockets               │
│  ┌────────────────▼────────────────────────┐  │
│  │  sim_bridge (Python)                    │  │
│  │  ├── 接收电机指令 → 更新动力学模型        │  │
│  │  ├── 动力学模型 step → 生成虚拟传感器      │  │
│  │  ├── 发送虚拟 IMU / 电机反馈 / 遥控数据   │  │
│  │  ├── 场景管理 (YAML 定义)                │  │
│  │  └── 报告输出 (JSON)                     │  │
│  └─────────────────────────────────────────┘  │
└───────────────────────────────────────────────┘
```

---

## 三、分阶段实施规划

### P1：SITL 基础设施（优先级最高）

**目标**：固件编译为 Linux 进程，能启动所有 FreeRTOS task。

#### 交付物

| 文件 | 说明 |
|---|---|
| `cmake/toolchains/linux-gcc.cmake` | Linux 本地 GCC 工具链 |
| `CMakeLists.txt` 新增 SITL target | `balance_chassis_sitl` 可执行目标 |
| `third_party/freertos_port_linux/` | FreeRTOS 官方 Linux/POSIX 端口 |
| `runtime/bsp/sitl/bmi088_sitl.c` | IMU 桩：从 UDP socket 读虚拟 accel/gyro |
| `runtime/bsp/sitl/fdcan_sitl.c` | CAN 桩：从 UDP socket 收发虚拟电机帧 |
| `runtime/bsp/sitl/uart_sitl.c` | UART 桩：从 UDP socket 读虚拟 DBUS 帧 |
| `runtime/bsp/sitl/dwt_sitl.c` | DWT 桩：用 `clock_gettime()` 替代 |
| `runtime/bsp/sitl/pwm_sitl.c` | PWM 桩：空实现 |
| `runtime/bsp/sitl/hal_stub.c` | 最小 HAL 桩（`HAL_GetTick()` 等） |
| `sim/bridge/sim_bridge.py` | Python 仿真桥：管理 socket 连接 + 时间推进 |

#### 验收标准
- `cmake --build . --target balance_chassis_sitl` 编译成功
- `./balance_chassis_sitl` 能启动，5 个 FreeRTOS task 全部创建
- `sim_bridge.py` 能通过 socket 喂静态 IMU 数据，固件能读到并运行 Mahony

---

### P2：Replay 日志回放

**目标**：在板上记录一段真实数据，在 SITL 中回放，对比输出。

#### 交付物

| 文件 | 说明 |
|---|---|
| `runtime/module/log/log_format.h` | 二进制日志帧格式定义 |
| `runtime/module/log/log_writer.c` | 板上日志写入（旁路 hook） |
| `sim/replay/log_reader.py` | PC 端日志读取器 |
| `sim/replay/replay_runner.py` | 回放运行器：读日志 → 按时间喂 SITL → 收集输出 |
| `sim/replay/baseline_compare.py` | 基线对比：计算输出漂移 → pass/fail |

#### 日志帧格式（初版）
```
┌─────────┬──────────┬─────────┬──────────────────┐
│ sync(2B) │ time(4B) │ id(2B)  │ payload(N bytes) │
│ 0xAA55   │ tick_ms  │ topic_id│ topic 原始数据    │
└─────────┴──────────┴─────────┴──────────────────┘
```

#### 验收标准
- 板上能记录至少 10 秒的 `ins_data` + `chassis_state` + `leg_output` 日志
- PC 端 `replay_runner.py` 读日志喂 SITL，SITL 输出与日志记录的原始输出偏差 < 1%
- 自动生成 pass/fail 报告

---

### P3：物理仿真闭环

**目标**：用简化轮腿动力学模型替代日志数据源，实现闭环仿真。

#### 交付物

| 文件 | 说明 |
|---|---|
| `sim/physics/wheel_leg_model.py` | 轮腿动力学：5 连杆 + 倒立摆 + 轮地接触 |
| `sim/bridge/sim_bridge.py` 扩展 | 闭环模式：动力学 → 虚拟传感器 → 固件 → 电机指令 → 动力学 |
| `sim/scenarios/*.yaml` | 场景定义：静止平衡、推恢复、速度跟踪、跳跃 |
| `sim/reports/` | 每次仿真输出 JSON 报告 + 关键指标曲线 |

#### 验收标准
- 静止平衡场景：倾斜角 < 2°，持续 10 秒不倒
- 推恢复场景：5N 侧推后 2 秒内恢复平衡
- 速度跟踪场景：1 m/s 目标，跟踪误差 < 10%

---

### P4：Module step 化（补充测试手段）

**目标**：核心算法可独立编译为 `.so`，支持 PC 端单元测试。

#### 改造范围

| 文件 | 改动 |
|---|---|
| `VMC_calc.h` | 去 `#include "main.h"` 和 `#include "INS_task.h"` |
| `controller.h` | 去 `#include "main.h"`，用 `stdint.h` 替代 |
| `kalman_filter.h` | 去 HAL 依赖 |
| `mahony_filter.h` | 去 HAL 依赖 |
| `pid.h` | 确认无 HAL 依赖 |

#### 交付物
- `CMakeLists.txt` 新增 `balance_chassis_module.so` 共享库目标
- `tests/test_vmc.py` — VMC 正逆运动学单元测试
- `tests/test_lqr.py` — LQR 增益矩阵验证
- `tests/test_pid.py` — PID 阶跃响应测试

#### 验收标准
- `module.so` 在 Linux 上编译成功，无 HAL 符号依赖
- 3 个 Python 单元测试全部 pass

---

### P5：平台收敛

**目标**：清理过渡层，统一消息总线，接入 flash/debug。

#### 交付物

| 项 | 说明 |
|---|---|
| 消息总线统一 | `message_center` → `ARF topic`，`extern` 全局变量 → topic 订阅 |
| seed 层退场 | 合并 `balance_chassis_seed/` 到主 app |
| flash/debug | CLI 接入 `openocd` + `arm-none-eabi-gdb` |
| CI 集成 | SITL 场景 + module 单元测试作为 CI 回归 |

---

## 四、时间线建议

```
     P1: SITL 基础设施        P2: Replay         P3: Physics Sim
     ──────────────────    ──────────────     ──────────────
     [  1~2 周  ]           [  1 周  ]         [  1~2 周  ]

                            P4: Module step 化
                            ──────────────
                            [  1 周  ]

                                               P5: 平台收敛
                                               ──────────────
                                               [  持续  ]
```

**P1 是最高优先级**——它是 P2 和 P3 的前置依赖。P4 可以和 P1 并行做（不同层的改造，不冲突）。P5 是长期持续的事。

---

## 五、CLI 命令规划（最终形态）

```bash
# 生成
platform generate                    # CubeMX CLI 生成底层代码

# 构建
platform build                       # 构建 hw 固件 (arm-none-eabi-gcc)
platform build sitl                  # 构建 SITL 可执行文件 (linux-gcc)
platform build module                # 构建 module.so 共享库

# 验证
platform sim standstill              # 运行 SITL 仿真场景
platform replay drive_test.bin       # 回放日志
platform test                        # 运行全部单元测试 + SITL 场景

# 部署
platform flash                       # 烧录固件到板子
platform debug                       # 启动 GDB 调试会话
```

---

## 六、参考项目

| 参考项目 | 参考内容 |
|---|---|
| PX4 | SITL lockstep 架构、uORB pub/sub 设计 |
| Betaflight | SITL HAL 桩实现、Blackbox 日志格式 |
| iNav | HITL 备选路线参考 |
| HoneyGrant/STM32_RoboMaster | BSP/Module/App 三层设计验证 |
