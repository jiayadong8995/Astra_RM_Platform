# Phase 2: Host Safety Control Verification - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-03-31
**Phase:** 2-Host Safety Control Verification
**Areas discussed:** phase definition, authoritative path, injection seams, observation seams, fail oracles, wheel-leg coupling scope

---

## Phase-defining priority

| Option | Description | Selected |
|--------|-------------|----------|
| 把当前主控制链变成可注入、可观测、可判错的安全验证链 | 围绕当前真实主链建立可验证的安全闭环 | ✓ |
| 先继续扩平台或抽象 | 优先做更通用的平台化工作，再回头补安全验证 | |

**User's choice:** Phase 2 的重点应该是“把这条已经存在的主控制链，变成可注入、可观测、可判错的安全验证链”。  
**Notes:** 用户明确确认这比继续抽象平台更重要。

---

## 权威验证路径

| Option | Description | Selected |
|--------|-------------|----------|
| 验证当前真实 task + topic 主链 | 直接验证当前 `remote/observe/chassis/motor_control` 主路径 | ✓ |
| 绕开当前主链，只测理想化 direct interface 路径 | 先做更纯的架构路径，再映射回现实现实 | |

**Agent recommendation adopted:** 先验证当前真实主链，而不是先构造一条更“理想”的旁路测试链。  
**Notes:** 这样验证通过的结果才对应当前实际上机会用到的实现。

---

## 输入注入策略

| Option | Description | Selected |
|--------|-------------|----------|
| 全部从 device/profile seam 注入 | 所有 fake 输入都从设备层进入 | |
| 全部从 message/topic seam 注入 | 所有场景都从 transport ingress 进入 | |
| 分层注入：设备输入走 device/profile，链路故障走 message/topic | 根据问题性质选择更贴近当前实现的注入点 | ✓ |

**Agent recommendation adopted:**  
- 传感器、遥控输入从 `device/profile seam` 注入  
- 链路丢失、stale、transport 类故障从 `message/topic` 或等价 runtime ingress seam 注入  

**Notes:** 用户未反对，允许按推荐默认值继续收口。

---

## 观测点范围

| Option | Description | Selected |
|--------|-------------|----------|
| 先只盯最终安全输出与关键 enable 位 | `actuator_command` + `start/control_enable/actuator_enable` | ✓ |
| 一开始铺开大量中间状态观测 | 先把所有关键中间量都暴露出来 | |

**Agent recommendation adopted:** 先把观测点收敛到 `actuator_command` 和 enable/start 相关关键状态。  
**Notes:** Phase 2 目标是先建立硬判错能力，不是先铺满 instrumentation。

---

## fail oracle 风格

| Option | Description | Selected |
|--------|-------------|----------|
| 硬判据 | 方向错、映射错、超限、stale 输入仍输出、非法状态切换等直接 fail | ✓ |
| 软判断 | 先看趋势或人工判断“像不像安全行为” | |

**Agent recommendation adopted:** 使用硬判据。  
**Initial oracle set:**  
- 非法方向/映射：直接 fail  
- 超限输出：直接 fail  
- stale/invalid/unavailable sensor 下仍有 enabled 输出：直接 fail  
- link 丢失或 stale command 下未进入安全行为：直接 fail  
- 非法状态切换后仍允许闭环输出：直接 fail  

---

## SAFE-06 范围

| Option | Description | Selected |
|--------|-------------|----------|
| 先做窄而明确的危险签名回归 | 先抓 1 到 2 个明确 wheel-leg danger signatures | ✓ |
| 一开始追求更完整的轮腿耦合稳定性证明 | 把 broad stability proof 放进本 phase | |

**Agent recommendation adopted:** 先只做窄而明确的危险签名回归。  
**Notes:** 这样能守住 Phase 2 范围，避免一开始把“控制安全验证”做成开放式研究。

---

## 当前现实约束

| Observation | Impact |
|------------|--------|
| `verify phase1` 当前能通过并观察到 `actuator_command` | 说明最小活路径已存在，可作为 Phase 2 起点 |
| 单独 `sim` 命令当前仍可能出现 `bridge_startup_error` | 说明 standalone sim 还不应被视为 Phase 2 的唯一权威入口 |
| 主链当前仍通过 `message_center` 在任务间传递 `robot_intent`、`device_feedback`、`actuator_command` | Phase 2 必须验证当前现实主链，而不是假设文档目标架构已达成 |

## the agent's Discretion

- Phase 2 可混合使用 host C tests 与少量 Python orchestration，只要不绕开当前真实主链
- 第一批 wheel-leg danger signature 的具体案例可由 planning/research 细化
- Phase 2 机器可读 verdict artifact 的字段可在 planning 阶段设计

## Deferred Ideas

- Fake-link 证据层与通信/控制故障分离的完整形态放到 Phase 3
- 平台 ownership 收口与 direct-interface 架构整改放到 Phase 4
- 更广泛的 replay / trace 回归能力放到后续阶段
