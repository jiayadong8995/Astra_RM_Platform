# Astra RM Robot Platform

## What This Is

This project is a Robotmaster-oriented embedded robot platform, with the current platform direction centered on a wheeled-legged robot (`balance_chassis`) as the first real integration target. It aims to unify firmware generation, cross-target builds, host-side verification, and controlled bring-up so the team can evolve robot control software with confidence instead of relying on risky first-on-robot debugging.

## Core Value

Make wheeled-legged Robotmaster control software safe to evolve by catching dangerous control and data-link errors before the robot ever gets a chance to go unstable on hardware.

## Current State (v2 shipped)

v2 is complete. The platform has been simplified from 5-6 layers to a clean 4-layer architecture (bsp → control → app → module):

- device_layer 间接层已完全删除（47 文件，1139 行），BSP 端口接口用链接时多态替代运行时 vtable
- topic wrapper 样板代码已去掉（8 文件），任务直接调用 PubRegister/SubGetMessage，topic 名称集中在 topics.h
- 两种并行命令类型已统一为 indexed joints[N]/wheels[N]，消除了 160 行映射代码
- 控制代码到硬件的间接跳数从 5 降到 2
- v1 的全部安全验证能力保留：10 个 CTest 目标、validate 流水线 5 阶段、SAFE-01..06 安全门控
- CMake 用 interface library 去除了重复配置

The developer inner loop is: `python3 -m robot_platform.tools.platform_cli.main validate`

## Next Milestone Goals (v3)

v3 方向指向受限硬件上车：

- 开发者可以将验证通过的变更推进到受限硬件上车流程
- 轮腿机器人可以在受限条件下完成最小闭环上车
- 硬件上车产物可以与之前的 host/fake-link 证据关联
- 回放存储的 SITL 或硬件 trace 进行回归测试
- 更丰富的故障注入场景
- 平台抽象在第二个机器人 profile 上得到验证

## Requirements

### Validated (v1 + v2)

All 24 v1 requirements satisfied. See [v1 archive](.planning/milestones/v1-REQUIREMENTS.md).
All 12 v2 requirements satisfied. See [v2 archive](.planning/milestones/v2-REQUIREMENTS.md).

### Active

*No active requirements. Run `/gsd:new-milestone` to define v3 requirements.*

### Out of Scope

- 全面竞赛功能 — 当前优先级是安全上车
- 高保真仿真 — sim 继续作为逻辑验证工具
- 在上车前扩展到多个机器人 profile — 先在 balance_chassis 上完成硬件闭环

## Constraints

- **Platform direction**: 保持可复用平台方向，4 层架构已验证
- **Safety**: 硬件上车必须通过 v1+v2 建立的验证门控
- **Validation model**: 分级验证 — host tests → fake-link → SITL smoke → 受限硬件
- **Architecture**: 4 层结构（bsp → control → app → module），BSP 端口 + 链接时多态
- **Build environment**: CMake + CubeMX + Python CLI，工具链不变
- **Current target**: balance_chassis 继续作为证明路径

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Keep the project oriented around a reusable platform | Generality preferred even at higher upfront cost | v1 Validated |
| Split success criteria into staged maturity levels | v1 host-side, v2 simplification, v3 hardware | v2 Validated |
| Treat host-side TDD and fake data-link verification as first-class requirements | Primary blocker was lack of trust before hardware bring-up | v1 Validated |
| Define "safe to bring up" by explicit failure modes to prevent | Block inverted outputs, broken limits, invalid transitions, stale-link control, unstable coupling | v1 Validated |
| 适度瘦身而非激进重写 | 保留平台方向和验证能力，去掉不必要的间接层 | v2 Validated |
| message_center 收窄到必要场景 | 6 个 topic 都是真正跨任务的，去掉的是包装代码不是总线 | v2 Validated |
| 去掉 device_layer，用 BSP 端口 + 链接时多态替代 | 运行时 vtable 在单 profile 场景下是纯间接层 | v2 Validated |

## Evolution

This document evolves at phase transitions and milestone boundaries.

---
*Last updated: 2026-04-02 after v2.0 milestone completion*
