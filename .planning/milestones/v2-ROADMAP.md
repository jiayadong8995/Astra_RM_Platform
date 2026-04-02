# v2 Roadmap Archive: Astra RM Robot Platform

**Milestone:** v2.0 — Platform Simplification
**Completed:** 2026-04-02
**Phases:** 4 (10 plans)
**Requirements:** 12/12 satisfied

## Overview

缩减平台代码复杂度，从 5-6 层精简到 4 层（bsp → control → app → module），去掉 device_layer 间接层和 topic wrapper 样板代码，同时保持 v1 建立的安全验证能力。

## Key Accomplishments

1. 统一两种并行命令类型为 indexed joints[N]/wheels[N]，消除 160 行映射代码
2. 引入 BSP 端口接口（ports.h），用链接时多态替代运行时 vtable dispatch
3. 迁移所有测试和控制任务到 BSP 端口路径，ports_fake 提供测试注入
4. 删除整个 runtime/device/ 目录（47 文件，1139 行间接层）
5. 去掉 8 个 topic wrapper 文件，创建集中的 topics.h + 提取 readiness gates
6. generated/ 移入 bsp/boards/，CMake interface library 去重，validate 流水线全绿

## Complexity Reduction

| Metric | Before (v1) | After (v2) | Delta |
|--------|-------------|------------|-------|
| Runtime layers | 5-6 | 4 | -2 |
| Device layer files | 29+ | 0 | -29 |
| Topic wrapper files | 10 | 0 | -10 |
| Command mapping LOC | ~160 | 0 | -160 |
| Indirection depth | 5 hops | 2 hops | -3 |

## Phases

| Phase | Plans | Completed |
|-------|-------|-----------|
| 1. Port Foundation | 2/2 | 2026-04-02 |
| 2. Seam Migration | 2/2 | 2026-04-02 |
| 3. Device Layer Removal | 3/3 | 2026-04-02 |
| 4. Consolidation | 3/3 | 2026-04-02 |

---
*Archived: 2026-04-02*
