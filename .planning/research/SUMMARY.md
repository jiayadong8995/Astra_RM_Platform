# v2 Research Summary: Platform Simplification

**Project:** Astra RM Robot Platform
**Domain:** Embedded robot platform simplification (5-6 layers → 4 layers)
**Researched:** 2026-04-01
**Confidence:** HIGH

## Executive Summary

v1 建立了可信的验证闭环，但代码架构过度设计。v2 的核心任务是瘦身：去掉 device_layer 间接层（29 文件 2806 行），收窄 message_center 包装代码（10 文件 329 行），统一冗余的命令类型（消除 160 行映射），最终从 5-6 层精简到 4 层（bsp → control → app → module）。

三个研究维度高度一致：device_layer 是纯间接层，所有 6 个 pub/sub topic 都是真正跨任务的（总线本身不需要去掉，只需去掉包装），测试接缝迁移必须在任何结构变更之前完成。

## Key Findings

### Stack (STACK.md)
- 工具链不变：C11 + CMake + Python CLI + FreeRTOS + STM32CubeMX
- 去掉 device_layer aggregate + profile binding（30 文件 ~430 LOC 间接层）
- 去掉 device semantic wrappers（14 文件 vtable 包装）
- 统一两种并行命令类型，消除 actuator_gateway 和 device_layer 中的 160 行映射
- message_center 保留，但去掉 10 个 topic wrapper 文件，改为直接调用 + 集中 topics.h
- 用链接时多态（CMake 选择不同 .c 文件）替代运行时 vtable dispatch

### Features (FEATURES.md)
- Table stakes: 去掉 device 间接层、去掉 topic wrapper、收窄 pub/sub、精简到 4 层、保留 11 个 CTest 目标、保留 HW/SITL 后端选择、保持 validate 流水线
- Differentiators: 降低认知负担、直接 struct 传递、测试 stub 简化、contracts 成为稳定 API 边界
- Anti-features: 不重写 message_center、不引入新抽象层替代 device_layer、不去掉 SITL 能力、不合并所有任务到 super-loop、不在简化期间加新功能

### Architecture (ARCHITECTURE.md)
- 目标结构：bsp/（含 ports.h 接口定义）→ control/（含 topics.h 常量）→ app/ → module/
- BSP 端口模式：`platform_imu_read()`, `platform_remote_read()`, `platform_motor_write_command()` — 简单函数签名，CMake 选择实现
- 数据流从 5 跳减到 2 跳：control → BSP port → BSP implementation
- generated/ 移入 bsp/boards/ 下

### Pitfalls (PITFALLS.md)
- **CRITICAL**: 7/11 CTest 目标依赖 `platform_device_set_test_hooks` — 必须先迁移测试接缝
- **CRITICAL**: `wait_ready` 启动门控嵌在 topic wrapper 里 — 去掉 wrapper 前必须提取
- **CRITICAL**: 简化不能变成重写 — 严格限定每个 phase 的文件范围
- **HIGH**: HW/SITL 后端区分目前在一个地方（device_layer.c）— 替代方案必须保持"一处决定"原则
- **MODERATE**: CMakeLists.txt 989 行，结构变更时容易遗漏更新

## Migration Order (Risk-Minimizing)

所有研究一致推荐的顺序：

1. **统一命令类型** — 纯重构，零行为变化
2. **引入 BSP 端口接口** — 新旧并存
3. **迁移测试接缝** — 用链接时 BSP 端口 fake 替代 device_layer test hooks
4. **迁移 control 代码到端口** — ins_task, remote_task, motor_control_task
5. **提取启动门控** — 从 topic wrapper 移到独立函数
6. **删除 device layer** — 29 文件
7. **精简 topic wrapper** — 10 文件 → 1 个 topics.h
8. **扁平化目录** — generated 移入 BSP，更新 CMake

关键不变量：`test_balance_safety_path` 在每一步都不能变红。

## Complexity Reduction Targets

| Metric | Before (v1) | After (v2) | Delta |
|--------|-------------|------------|-------|
| Runtime layers | 5-6 | 4 | -2 |
| Device layer files | 29 | 0 | -29 |
| Topic wrapper files | 10 | 1 | -9 |
| Command mapping LOC | ~160 | 0 | -160 |
| Indirection depth | 5 hops | 2 hops | -3 |
| Max directory nesting | 4 levels | 2 levels | -2 |

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | 工具链不变，变更范围明确 |
| Features | HIGH | 基于代码行数和文件计数的具体基线 |
| Architecture | HIGH | 目标结构匹配 basic_framework 已验证的 4 层模式 |
| Pitfalls | HIGH | 所有风险基于直接代码分析和测试依赖审计 |

**Overall confidence:** HIGH

## Sources

- `.planning/research/STACK.md` — 技术栈分析和迁移策略
- `.planning/research/FEATURES.md` — 特性景观和结果指标
- `.planning/research/ARCHITECTURE.md` — 架构变更和目录结构
- `.planning/research/PITFALLS.md` — 12 个陷阱和风险缓解
- `references/external/basic_framework` — 4 层参考架构
- `references/external/StandardRobotpp` — 直接状态共享参考
- `references/XRobot` — 扁平模块参考
- `references/external/ros2_control_demos`, `legged_control` — 接口标准化参考

---
*Research completed: 2026-04-01*
*Ready for requirements: yes*
