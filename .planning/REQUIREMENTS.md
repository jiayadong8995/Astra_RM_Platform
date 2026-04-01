# Requirements: Astra RM Robot Platform v2.0

**Defined:** 2026-04-01
**Core Value:** 缩减平台代码复杂度，让架构更直接、更易理解、更易测试，同时保持 v1 建立的安全验证能力。

## v2 Requirements

### 结构简化

- [ ] **SLIM-01**: 去掉 `device_layer`/`device_profile` 间接层，用 BSP 端口接口 + 链接时多态替代运行时 vtable dispatch
- [ ] **SLIM-02**: 去掉 `device/imu`、`device/remote`、`device/actuator` 下的 semantic wrapper vtable 文件，驱动逻辑归入 BSP
- [ ] **SLIM-03**: 去掉 `*_topics.c/h` wrapper 样板代码（10 文件），任务直接调用 PubRegister/SubGetMessage，topic 名称集中到一个 `topics.h`
- [ ] **SLIM-04**: 统一 `platform_actuator_command_t` 和 `platform_device_command_t` 为一种命令类型，消除 actuator_gateway 和 device_layer 中的字段映射代码
- [ ] **SLIM-05**: runtime 层级从 5-6 层精简到 4 层（bsp → control → app → module），`generated/` 移入 `bsp/boards/` 下

### 验证保持

- [ ] **KEEP-01**: 所有 v1 安全门控（SAFE-01..06）在简化后继续通过，等效 CTest 覆盖不丢失
- [ ] **KEEP-02**: `validate` CLI 流水线 5 阶段（build_sitl → host_tests → python_tests → smoke → verify_phase3）继续工作
- [ ] **KEEP-03**: HW 和 SITL 双后端构建能力保留，后端选择从运行时 vtable 改为编译时/链接时选择
- [ ] **KEEP-04**: host-side 测试接缝从 device_layer test hooks 迁移到链接时 BSP 端口 fake，测试注入能力等效保留

### 代码质量

- [ ] **QUAL-01**: 控制代码到硬件的间接跳数从 5 降到 2（control → BSP port → BSP implementation）
- [ ] **QUAL-02**: runtime 下最大目录嵌套从 4 层降到 2 层
- [ ] **QUAL-03**: CMakeLists.txt 去除重复的 include 路径和 sanitizer 配置，使用 CMake interface library 统一

## v3 Requirements (Deferred)

### 硬件上车

- **HW-01**: 开发者可以将验证通过的变更推进到受限硬件上车流程
- **HW-02**: 轮腿机器人可以在受限条件下完成最小闭环上车
- **HW-03**: 硬件上车产物可以与之前的 host/fake-link 证据关联

### 扩展验证

- **VAL-01**: 开发者可以回放存储的 SITL 或硬件 trace 进行回归测试
- **VAL-02**: 验证包含更丰富的传感器、数据链路和执行器故障注入场景

### 平台复用

- **PLAT-01**: 平台抽象在 balance_chassis 之外至少一个机器人 profile 上得到验证
- **PLAT-02**: 平台可以扩展机器人特定控制功能而不重新引入跨层所有权混乱

## Out of Scope

| Feature | Reason |
|---------|--------|
| 硬件上车和真机闭环 | 推到 v3，v2 专注代码简化 |
| 新增机器人 profile | 先在 balance_chassis 上证明简化后的架构 |
| 重写 message_center 内部实现 | 总线实现正确且经过测试，问题在包装代码不在核心 |
| 引入新抽象层替代 device_layer | 目标是减少层级，不是换一个名字的间接层 |
| 合并 FreeRTOS 任务到 super-loop | 任务分离有实时调度意义，不应合并 |
| 在简化期间添加新功能 | v2 是纯重构里程碑，新功能属于 v3 |
| 高保真仿真 | sim 继续作为逻辑验证工具 |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| SLIM-01 | — | Pending |
| SLIM-02 | — | Pending |
| SLIM-03 | — | Pending |
| SLIM-04 | — | Pending |
| SLIM-05 | — | Pending |
| KEEP-01 | — | Pending |
| KEEP-02 | — | Pending |
| KEEP-03 | — | Pending |
| KEEP-04 | — | Pending |
| QUAL-01 | — | Pending |
| QUAL-02 | — | Pending |
| QUAL-03 | — | Pending |

**Coverage:**
- v2 requirements: 12 total
- Mapped to phases: 0 (pending roadmap)
- Unmapped: 12

---
*Requirements defined: 2026-04-01*
