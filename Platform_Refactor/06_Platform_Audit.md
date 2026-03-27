# 机器人开发平台 — 逐块审计

> 以"你想要一套机器人开发平台"为锚点，把**需求、已有设计、当前实现、差距、建议路径**逐块对齐。

---

## 全景：一套机器人开发平台需要什么

```
┌─────────────────────────────────────────────────────────────────────┐
│                        机器人开发平台                                │
├─────────┬─────────┬────────┬─────────┬──────────┬──────────────────┤
│ ① 生成链 │ ② 构建链 │ ③ 运行时 │ ④ 验证链  │ ⑤ 部署链  │ ⑥ 多项目管理    │
│ config   │ CMake   │ 四层分离│ replay  │ flash    │ 多板/多机型     │
│ → code   │ + GCC   │ 架构   │ + sim   │ + debug  │ 统一入口        │
└─────────┴─────────┴────────┴─────────┴──────────┴──────────────────┘
```

下面按 6 个子系统逐块展开。

---

## ① 生成链 (Codegen)

### 需求
| 项 | 描述 |
|---|---|
| 核心能力 | 从配置文件驱动生成 MCU 底层代码（时钟、外设、中断、启动文件） |
| 第一阶段 | 加载现有 `.ioc`，用 CubeMX CLI 生成 |
| 第二阶段 | `yaml config → board model → generator backend → generated tree` |
| 后端可换 | 当前 CubeMX 6.x，未来可切 CubeMX2 |
| 输出规则 | 禁止手写业务逻辑，可被再生成覆盖 |

### 当前设计
- `01_Architecture.md` §4.1 定义了生成后端抽象
- 预留 `stm32cubemx_ioc_backend` 和 `cubemx2_backend` 两条路线
- `phase0_codegen_first.md` 定义了先跑通 CubeMX CLI 的前置条件

### 当前实现

| 项 | 状态 |
|---|---|
| CubeMX 6.17.0 CLI 调用 | ✅ 已验证 |
| `.ioc` 加载 + 代码生成 | ✅ `config load + generate code` |
| CLI 命令 `generate` | ✅ `main.py → _generate_balance_chassis()` |
| 输出固定目录 | ✅ `runtime/generated/stm32h7_ctrl_board_raw/` |
| 可重复生成 | ✅ 已验证 |

### 差距

| 项 | 差距 | 优先级 |
|---|---|---|
| yaml 驱动生成 | 未实现，当前仍直接用 `.ioc` 路径 | P2 (可以后做) |
| 后端抽象层 | 未实现 `board model → backend` 接口 | P2 |
| 多板支持 | 只支持 `balance_chassis` 一块板 | P3 |
| `_raw` → 正式目录转换 | 未做 | P3 |

### 建议路径

> **当前实现已够用。** 生成链目前最大的价值是"CubeMX CLI 已跑通"。`yaml → board model → backend` 这条线是平台成熟期的事，不要现在做。当前 `.ioc` 直接加载的方式足够支撑 Chassis 迁移全过程。

---

## ② 构建链 (Build)

### 需求
| 项 | 描述 |
|---|---|
| 核心能力 | 脱离 Keil，用 CMake + arm-none-eabi-gcc 生成固件 |
| 输出 | `.elf` + `.bin` + `.map` |
| 多固件目标 | seed (最小), legacy_full (完整底盘) |

### 当前实现

| 项 | 状态 |
|---|---|
| CMakeLists.txt | ✅ 379 行，结构完整 |
| GCC toolchain 文件 | ✅ `cmake/toolchains/arm-none-eabi-gcc.cmake` |
| Linker script | ✅ `cmake/linker/stm32h723_flash.ld` |
| FreeRTOS GCC 端口 | ✅ `third_party/freertos_port_gcc_cm7_r0p1/` |
| seed ELF | ✅ `balance_chassis_hw_seed.elf` |
| legacy full ELF | ✅ `balance_chassis_legacy_full.elf` |
| CLI `build` 命令 | ✅ 4 个子模式 (hw_elf / hw_seed / legacy_obj / legacy_full) |
| Ninja 生成器 | ✅ |

### 差距

| 项 | 差距 | 优先级 |
|---|---|---|
| 编译器警告治理 | 未知当前 warning 数量 | P2 |
| 增量构建效率 | 未验证大规模修改后的重编译时间 | P3 |
| 多目标板 | CMakeLists 只写死了 balance_chassis | P3 |

### 建议路径

> **构建链已经基本成立。** 这是目前最成功的部分。短期不需要再投入。后续接入 Gimbal 时再做多目标扩展。

---

## ③ 运行时架构 (Runtime)

### 需求
| 项 | 描述 |
|---|---|
| 核心能力 | 四层分离：`generated → bsp → module → app`，依赖方向单向 |
| 关键约束 | module 禁止依赖 HAL/中断/RTOS；app 禁止直接碰寄存器 |
| osal | 提供 OS 抽象，使 module 可跑在 hw / replay / sim 三种环境 |
| 消息总线 | 统一的 pub/sub 数据通道，替代全局变量 |

### 当前实现

| 层 | 目录 | 状态 | 问题 |
|---|---|---|---|
| generated | `runtime/generated/stm32h7_ctrl_board_raw/` | ✅ 已填充 | — |
| bsp/boards | `runtime/bsp/boards/stm32h7_ctrl_board/` | ✅ 已迁移 | 边界未硬切 |
| bsp/devices | `runtime/bsp/devices/{bmi088,dm_motor,remote_control}` | ✅ 已迁移 | 边界未硬切 |
| module | `runtime/module/{algorithm,controller,lib,message_center}` | ✅ 已迁移 | **仍依赖 HAL 头文件** |
| app | `runtime/app/balance_chassis/` | ✅ 已迁移 | **仍使用全局变量** |
| osal | `runtime/osal/` | ❌ 只有 README | — |

### 核心差距分析

#### 差距 1：module 并未真正脱离 HAL

当前 `module/algorithm/VMC/VMC_calc.h` 第一行：
```c
#include "main.h"        // ← 直接拉入 HAL
#include "INS_task.h"    // ← 直接拉入 app 层
```

这意味着 **module 层当前无法脱离板卡独立编译**。这是架构设计中最核心的约束，目前被完全违反。

#### 差距 2：三套消息机制未收敛

| 机制 | 位置 | 当前状态 |
|---|---|---|
| 全局变量 (`extern INS`, `extern chassis_move`) | app 层 | 大量使用中 |
| `message_center` (动态注册 pub/sub) | `module/message_center/` | chassis_task 中注册使用 |
| `ARF topic` (静态零拷贝 pub/sub) | `Framework/arf_topic.h` | 未被 legacy 代码使用 |

三者**在同一控制链中并存**，且语义重叠。

#### 差距 3：osal 空壳

`osal` 是实现 `module` 跨环境运行的关键。没有它：
- module 无法在 replay 中运行
- module 无法在 sim 中运行
- 依赖方向约束无法被技术手段强制

#### 差距 4：app 层 = legacy 代码直接复制

`freertos_legacy.c` 表明当前 app 层做法是：
1. 把 CubeMX 生成的 `freertos.c` 替换成手写版
2. 直接调用 `INS_task()` / `Chassis_task()` 等全局函数
3. 不经过任何 app lifecycle / mode manager

这实质上是一个垫片层，而不是真正的 app 架构。

### 建议路径

> 运行时架构是**当前最大的技术债**。目录已经搬好了，但分层约束和依赖方向还没有被真正强制。
>
> 建议按以下顺序逐步推进，**每一步都不改变运行行为**：

**Step 1：osal 最小实现**
- 只需 4 个接口：`osal_delay_ms`, `osal_get_tick`, `osal_enter_critical`, `osal_exit_critical`
- hw 实现直接转发到 FreeRTOS
- replay/sim 实现用 host 的 sleep/clock

**Step 2：module 去 HAL 依赖**
- `VMC_calc.h` 去掉 `#include "main.h"`，改用 `stdint.h` + 自定义类型
- `VMC_calc.h` 去掉 `#include "INS_task.h"`，改用前向声明或 module 内部类型
- 验证 module 可在无 HAL 环境下独立编译

**Step 3：消息总线收敛**
- 选定 `ARF topic` 或 `message_center` 之一作为唯一主线
- 将 `chassis_task.c` 中的 `extern INS` 等全局变量替换为显式 topic 订阅
- 建议选 `ARF topic`（零动态分配、编译期静态定义更适合嵌入式）

**Step 4：freertos_legacy → app lifecycle**
- 引入简单的 `app_init()` / `app_step()` / `app_mode_switch()` 钩子
- task 线程不再直接调用全局函数，而是调用 app 编排入口

---

## ④ 验证链 (Verification)

### 需求
| 项 | 描述 |
|---|---|
| replay | 读取实车日志，按时间推进，喂传感器数据给 module，对比输出行为 |
| physics_sim | 简化动力学闭环，控制器在 sim 中运行，场景化回归 |
| 行为基线 | 迁移前后行为一致性 → pass/fail 报告 |

### 当前实现

| 项 | 状态 | 详情 |
|---|---|---|
| replay 目录 | ❌ 只有 README (51 bytes) | 无日志格式、无读取器、无比对 |
| physics sim | ⚠️ 最小 demo | `simple_balance.py` = 76 行独立倒立摆模型 |
| sim runner | ⚠️ 可运行 | `runner.py` 加载 YAML 场景，跑仿真，输出 JSON 报告 |
| sim 场景 | ⚠️ 3 个场景 | `standstill`, `push_recovery`, `tilt_recovery` |
| sim adapter | ❌ 只有 README | 无 legacy 控制链接入 |

### 核心差距分析

#### 差距 1：replay = 空

这是审计报告里明确指出的**最大缺口**。没有 replay 意味着：
- 迁移正确性无从验证
- 重构后行为漂移无法检测
- 无法做回归测试

#### 差距 2：sim 不跑 legacy 控制链

当前 `simple_balance.py` 是一个**纯 Python 4 状态倒立摆**，控制器也是 Python 写的简化 PD。
它和你的 6 状态 LQR + VMC 五连杆**完全没有关系**。

`sim/adapters/` 是空白的，没有任何将 legacy C 控制链接入 Python sim 的桥接代码。

### 建议路径

#### replay（P0 最优先）

```
日志格式：binary log = [timestamp_us (u32) | topic_id (u16) | payload (N bytes)]
              ↓
日志采集：在板子上给每个 topic 的 publish 加一个旁路 log hook
              ↓
回放运行器：读日志文件 → 按时间推进 → 喂给 module step → 输出
              ↓
基线对比：离线比较原始日志输出 vs 重跑后输出 → pass/fail
```

**最小交付**：
1. 定义日志二进制格式 (8 字节头 + payload)
2. 在 `arf_topic_publish` 中加一个条件编译的旁路 `log_write()`
3. PC 端日志读取器 (Python)
4. 一条 `IMU → chassis_task → wheel_torque` 数据链的回放验证

#### physics_sim（P1 次优先）

当前 demo 的价值在于**框架已成型**（YAML 场景 + runner + JSON 报告）。
需要做的是把 plant 从 Python 倒立摆替换为**C 编译出的 shared library**：

```
C 控制链 (module) → 编译为 .so → Python ctypes 调用
              ↓
Python 仿真环境提供传感器模拟值 → 调用 C step → 收集输出
              ↓
YAML 场景定义 → runner 编排 → JSON 报告
```

这条路的前提是 **module 必须能脱离 HAL 独立编译**（回到 ③ 的 Step 2）。

---

## ⑤ 部署链 (Deploy)

### 需求
| 项 | 描述 |
|---|---|
| flash | 平台命令烧录固件到板子 |
| debug | 平台命令启动 GDB 调试会话 |

### 当前实现
- CLI 中 `flash` 和 `debug` 命令只是 `print("command placeholder: ...")`
- 无 OpenOCD / ST-Link / pyOCD 集成

### 建议路径

```python
# flash: 调用 openocd 或 st-flash
def _flash():
    subprocess.run([
        "openocd",
        "-f", "interface/stlink.cfg",
        "-f", "target/stm32h7x.cfg",
        "-c", "program balance_chassis_legacy_full.bin 0x08000000 verify reset exit"
    ])

# debug: 调用 openocd + arm-none-eabi-gdb
def _debug():
    # 后台启动 openocd server
    # 前台启动 gdb connect to :3333
```

**工作量**：约 50 行 Python，1~2 小时。

---

## ⑥ 多项目管理

### 需求
| 项 | 描述 |
|---|---|
| 多板支持 | 底盘板 (H723) + 云台板 (F4) + 未来更多 |
| 多机型 | balance_chassis, standard_infantry, sentry 等 |
| 项目配置 | `project.yaml` + `board.yaml` 驱动构建行为 |

### 当前实现
- `projects/balance_chassis/` 已有配置目录
- CMakeLists 只支持 `balance_chassis` 一个项目
- `Gimbal` 未迁移

### 建议路径

> 这是平台成熟期的事。当前只有一块板一个项目，不要过早做抽象。
> 等 Chassis 全链路（生成→构建→运行→验证→部署）全部走通后，再以 Gimbal 作为第二个项目来驱动多项目抽象。

---

## 总结：优先级矩阵

| 优先级 | 子系统 | 动作 | 理由 |
|---|---|---|---|
| **P0** | ④ replay | **立刻实现** | 没有 replay 则迁移正确性无法验证 |
| **P0** | ③ osal 最小实现 | **立刻实现** | replay 需要 osal 才能跑 module |
| **P1** | ③ module 去 HAL | **尽快做** | 阻塞 replay 和 sim 两条路 |
| **P1** | ⑤ flash/debug | **尽快做** | 工作量小 (~50 行)，解锁板上验证 |
| **P2** | ③ 消息总线收敛 | 渐进替换 | 风险可控，逐步进行 |
| **P2** | ④ physics sim 接入真实控制链 | 在 module 去 HAL 后做 | 前提是 module 可独立编译 |
| **P3** | ① yaml 驱动生成 | 延后 | 当前 .ioc 够用 |
| **P3** | ⑥ 多项目管理 | 延后 | 等第一个项目全链路通后再做 |
| **P3** | ③ app lifecycle | 延后 | 在消息总线收敛和 module 去 HAL 之后做 |

---

## 关键依赖链

```
osal 最小实现
     │
     ├──→ module 去 HAL 依赖
     │         │
     │         ├──→ replay 实现 ←──── P0 最优先
     │         │
     │         └──→ module 编译为 .so → sim 接入真实控制链
     │
     └──→ flash/debug 接入 (独立，无依赖)
                │
                └──→ 板上验证迁移固件
```

**一句话总结**：平台的生成链和构建链已经成立，现在的瓶颈是**运行时分层没有真正执行**（module 仍依赖 HAL），导致验证链无法启动。打通这个瓶颈点，后面的 replay、sim、多项目扩展才能顺畅推进。
