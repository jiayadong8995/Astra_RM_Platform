# Phase 1: Contracts and Verification Foundation - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-03-30
**Phase:** 1-Contracts and Verification Foundation
**Areas discussed:** 主机测试框架形态, message_center contract 安全策略, 验证结果与阶段报告格式, 生成物新鲜度检查

---

## 主机测试框架形态

| Option | Description | Selected |
|--------|-------------|----------|
| 最小 host C 闭环 + 薄 fake/stub 约定 | 先把主机侧 C 测试跑起来，只定义必要 seam，不做完整 fake 体系 | ✓ |
| 一开始建立完整 fake 体系 | 先把 fake 层整体抽象好，再开始扩测试面 | |

**User's choice:** 先把 C host test 最小闭环建起来，同时只定义很薄的 fake/stub 接口约定，不做完整 fake 体系。  
**Notes:** 用户最初提到 `message_center`、`device_layer`、`actuator_gateway`，但后续复核时明确指出这不符合 "smallest viable" 原则。修正后的锁定结论是：优先 `message_center` + `actuator_gateway`，`device_layer` 视 harness 成熟度决定是否纳入本 phase。

---

## message_center contract 安全策略

| Option | Description | Selected |
|--------|-------------|----------|
| 继续保留全局固定 64B 假设 | 依赖注释和使用习惯避免溢出 | |
| 保留固定上限但补更多检查 | 继续用统一 buffer，只增加校验 | |
| 按 topic 声明大小并做注册期校验 | 每个 topic 显式声明 payload size，注册时阻断不安全配置 | ✓ |

**User's choice:** 不要继续赌 `64B`，Phase 1 就改成按 topic 声明大小并做注册期校验。  
**Notes:** 用户明确要求不再接受“先赌不会超”的方案。

---

## 验证结果与阶段报告格式

| Option | Description | Selected |
|--------|-------------|----------|
| 单次总 JSON 报告，CLI 只做摘要 | 一个机器可读主报告作为唯一权威结果，终端输出做人类摘要 | ✓ |
| 总报告 + 每阶段独立细报告 | 设计更完整的分层报告体系 | |
| 先只做 CLI 文本输出 | 之后再补机器可读格式 | |

**User's choice:** Phase 1 直接统一成机器可读 JSON，CLI 只做人类友好的摘要。  
**Notes:** 后续跟进问题中，用户进一步确认报告粒度选择单次总报告，而不是总报告 + 分阶段细节。

---

## 生成物新鲜度检查

| Option | Description | Selected |
|--------|-------------|----------|
| 只告警，不阻断 | 允许继续生成和构建，但提示不新鲜 | |
| 软失败，可手动跳过 | 默认失败，但允许手动 override | |
| 硬阻断 | 一旦检测到 generated stale 就直接阻断 | ✓ |

**User's choice:** 先做硬阻断，不要只是告警。  
**Notes:** 用户希望把“不可信 firmware 输出”在 Phase 1 就做成硬门槛。

---

## 进一步收口（已修正）

| Option | Description | Selected |
|--------|-------------|----------|
| 模块范围：`message_center`、`actuator_gateway` 优先，`device_layer` 条件纳入 | 先守住最小闭环，再根据 harness 成熟度决定是否扩大 | ✓ |
| 模块范围一开始固定为三块 | `message_center`、`device_layer`、`actuator_gateway` 同时作为第一波硬范围 | |

**User's choice:** 优先 `message_center` + `actuator_gateway`，`device_layer` 视 harness 成熟度决定是否进入本 phase。  
**Notes:** 用户明确反对把 `device_layer` 在当前阶段写成第一波硬锁定范围，因为它 370 行，不是“薄 seam”。

---

## 最小活路径优先级

| Option | Description | Selected |
|--------|-------------|----------|
| 先证明最小活路径真的通 | 优先证明 `build sitl -> launch sitl -> bridge up -> inject one input -> observe one runtime output -> produce passed smoke report` | ✓ |
| 先继续围绕分层和 host harness 抽象推进 | 先把基础设施设计做完整，再看最小运行路径 | |

**User's choice:** 先证明 `build sitl -> launch sitl -> bridge up -> inject one input -> observe one runtime output -> produce passed smoke report` 这条最小链路真的通。  
**Notes:** 用户明确认为“框架到底能不能跑起来”是当前最应该优先确认的事情。

---

## the agent's Discretion

- 具体 host C test 库与轻量 fake/stub 组织形式
- 顶层 JSON 报告 schema 的具体字段
- stale generated artifacts 的检测机制与比较方式

## Deferred Ideas

- 完整 fake 体系 later 再扩
- 更重的分阶段详细报告 later 再评估
- 更广的控制器/任务层 host coverage 放到后续 phase
