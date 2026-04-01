# Astra RM Robot Platform

## What This Is

This project is a Robotmaster-oriented embedded robot platform, with the current platform direction centered on a wheeled-legged robot (`balance_chassis`) as the first real integration target. It aims to unify firmware generation, cross-target builds, host-side verification, and controlled bring-up so the team can evolve robot control software with confidence instead of relying on risky first-on-robot debugging.

## Core Value

Make wheeled-legged Robotmaster control software safe to evolve by catching dangerous control and data-link errors before the robot ever gets a chance to go unstable on hardware.

## Current State (v1 shipped)

v1 is complete. The platform now has:

- A trusted host-side verification loop with 11 CTest targets covering message transport, actuator gateway, 6 safety oracles (SAFE-01..06), SITL runtime bindings, and app startup
- Runtime-backed SITL fake-link adapters that drive the real control path and produce machine-readable artifacts distinguishing communication faults from control faults
- One authoritative `balance_chassis` bring-up path with explicit ownership boundaries between app composition, device adapters, and control logic
- A single `validate` CLI command that sequences build → host tests → python tests → smoke → verify → firmware with early-exit and closure artifact
- Safety gates that block inverted outputs, broken saturation, invalid arming, stale sensor/command data, and wheel-leg coupling instability before any on-robot attempt

The developer inner loop is: `python3 -m robot_platform.tools.platform_cli.main validate`

## Current Milestone: v2.0 Platform Simplification

**Goal:** 缩减平台代码复杂度，从 5-6 层过度设计的架构瘦身到 4 层清晰结构，同时保持 v1 建立的验证能力。

**Target features:**
- 去掉 device_layer/device_profile 间接层，BSP 驱动通过简单接口直接暴露给 control
- 收窄 message_center pub/sub 到真正需要跨任务解耦的场景，其余改为直接调用
- 从 5-6 层（generated→bsp→device→control→app→module）精简到 4 层（bsp→control→app→module），参考 basic_framework 的分层风格
- 测试和验证闭环跟着代码一起重构，允许临时断裂再修复
- 借鉴 ROS 2 的接口标准化思路，让控制器只认接口不认硬件实现

**Architecture direction:** 混合风格 — 靠近 basic_framework 的 4 层 + 克制的 pub/sub，同时借鉴 ROS 2 的接口标准化和 StandardRobotpp 的直接性。参考项目：references/external/basic_framework、references/external/StandardRobotpp、references/XRobot、references/external/ros2_control_demos、references/external/legged_control。

**Not in v2 scope:** 硬件上车推到 v3，v2 专注代码简化。

## Requirements

### Validated (v1)

All 24 v1 requirements satisfied. See [v1 requirements archive](.planning/milestones/v1-REQUIREMENTS.md).

### Active

- [ ] 去掉 device_layer/device_profile 间接层，BSP 适配器通过标准化接口直接服务 control
- [ ] message_center 收窄到必要的跨任务通信场景，非必要的 pub/sub 改为直接函数调用
- [ ] runtime 层级从 5-6 层精简到 4 层（bsp→control→app→module）
- [ ] 验证闭环（validate、host tests、verify）跟着重构更新，保持可用
- [ ] balance_chassis 在简化后的架构上继续作为证明路径

### Out of Scope

- 硬件上车和真机闭环 — 推到 v3，v2 专注代码简化
- 全面竞赛功能 — 当前优先级是架构健康度
- 新增机器人 profile — 先在 balance_chassis 上证明简化后的架构
- 高保真仿真 — sim 继续作为逻辑验证工具，不追求物理真实性

## Constraints

- **Platform direction**: 保持可复用平台方向，但简化不必要的抽象
- **Safety**: v1 的安全门控能力必须在简化后保留
- **Validation model**: 验证闭环允许重构但不允许永久退步
- **Architecture**: 目标是 4 层清晰结构，参考 basic_framework 和 ROS 2 接口模式
- **Build environment**: 构建工具链不变（CMake + CubeMX + Python CLI）
- **Current target**: balance_chassis 继续作为唯一证明路径

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Keep the project oriented around a reusable platform | The intended end state is a Robotmaster platform; generality preferred even at higher upfront cost | v1 Validated |
| Split success criteria into staged maturity levels (v1 host-side, v2 simplification, v3 hardware) | v2 专注架构简化，硬件上车需要简化后的代码基础 | v2 Active |
| Treat host-side TDD and fake data-link verification as first-class requirements | Primary blocker was lack of trust before hardware bring-up | v1 Validated |
| Define "safe to bring up" by explicit failure modes to prevent | Must block inverted outputs, broken limits, invalid transitions, stale-link control, unstable coupling | v1 Validated |
| Focused architecture review of platform weight and coupling | Implementation was overdesigned and too tightly coupled for effective TDD | v1 Validated |
| 适度瘦身而非激进重写 | 保留平台方向和验证能力，去掉不必要的间接层和过度抽象 | v2 Active |
| message_center 收窄到必要场景 | 只在真正需要跨任务解耦的地方用 pub/sub，其余直接调用更清晰 | v2 Active |
| 去掉 device_layer/device_profile 间接层 | 只有一个机器人的情况下，这层间接增加了不必要的复杂度 | v2 Active |

## Evolution

This document evolves at phase transitions and milestone boundaries.

---
*Last updated: 2026-04-01 after v2.0 milestone start*
