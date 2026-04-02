# v2 Requirements Archive: Astra RM Robot Platform

**Defined:** 2026-04-01
**Completed:** 2026-04-02
**Core Value:** 缩减平台代码复杂度，让架构更直接、更易理解、更易测试，同时保持 v1 建立的安全验证能力。

## 结构简化 — All Complete

- [x] **SLIM-01**: 去掉 device_layer/device_profile 间接层，用 BSP 端口接口 + 链接时多态替代 (Phase 3)
- [x] **SLIM-02**: 去掉 device semantic wrapper vtable 文件，驱动逻辑归入 BSP (Phase 3)
- [x] **SLIM-03**: 去掉 topic wrapper 样板代码，直接调用 PubRegister/SubGetMessage + topics.h (Phase 4)
- [x] **SLIM-04**: 统一两种并行命令类型，消除字段映射代码 (Phase 1)
- [x] **SLIM-05**: runtime 层级精简到 4 层，generated/ 移入 bsp/boards/ (Phase 4)

## 验证保持 — All Complete

- [x] **KEEP-01**: 所有 v1 安全门控（SAFE-01..06）在简化后继续通过 (Phase 2)
- [x] **KEEP-02**: validate CLI 流水线 5 阶段继续工作 (Phase 4)
- [x] **KEEP-03**: HW 和 SITL 双后端构建能力保留，链接时选择 (Phase 1)
- [x] **KEEP-04**: 测试接缝迁移到链接时 BSP 端口 fake (Phase 2)

## 代码质量 — All Complete

- [x] **QUAL-01**: 间接跳数从 5 降到 2 (Phase 3)
- [x] **QUAL-02**: 目录嵌套从 4 层降到 2 层 (Phase 4)
- [x] **QUAL-03**: CMake interface library 去重 (Phase 4)

## Coverage

- v2 requirements: 12 total
- Satisfied: 12
- Unsatisfied: 0

---
*Archived: 2026-04-02*
