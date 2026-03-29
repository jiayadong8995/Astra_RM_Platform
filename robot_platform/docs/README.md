# robot_platform docs

这里仅保留当前还能直接指导平台开发、验证和环境搭建的文档。

当前文档按两类管理：

- 正式文档：当前架构与代码必须遵守
- 参考文档：保留研究结论，但不作为当前唯一口径

## 正式文档

1. [platform_architecture.md](./platform_architecture.md)
   当前平台项目的统一架构说明，定义项目是什么、目标架构是什么、当前边界和演进目标是什么。
2. [project_roles_and_scope.md](./project_roles_and_scope.md)
   定义项目分工、执行主线、优先级和接口决策边界。
3. [generated_import_rules.md](./generated_import_rules.md)
   约束 `runtime/generated/` 的边界，避免把业务逻辑重新塞回生成目录。
4. [wsl_environment_setup.md](./wsl_environment_setup.md)
   记录当前仓库在 WSL 下跑通 `generate` / `build` 所需的环境前提。
5. [runtime_architecture_blueprint.md](./runtime_architecture_blueprint.md)
   当前运行时目标架构蓝图，固定顶层分层、主数据流和通信原则。
6. [runtime_architecture_task_list.md](./runtime_architecture_task_list.md)
   当前剩余任务清单，只保留仍未完成的收口项。
7. [runtime_contracts_draft.md](./runtime_contracts_draft.md)
   主控制链正式契约草案，定义 `DeviceInput / RobotState / RobotIntent / ActuatorCommand / DeviceCommand / DeviceFeedback`。
8. [runtime_capability_boundaries.md](./runtime_capability_boundaries.md)
   运行时能力边界表，定义哪些能力属于 `device / control / app / sim`，避免继续按 legacy task 拆系统。
9. [runtime_communication_matrix.md](./runtime_communication_matrix.md)
   运行时通信矩阵，固定哪些边界应使用直接接口，哪些场景才允许轻量 pub-sub。

## 参考文档

以下文档保留为参考研究，不再视为当前唯一口径：

1. [app_layer_architecture_research.md](./app_layer_architecture_research.md)
   记录 `app` 层拆分调研结论，可用于回看为什么放弃以 app 目录细拆为主线。
2. [sim_architecture_research.md](./sim_architecture_research.md)
   记录 `sim` 方向调研结论，可作为后续验证与 backend 设计的参考输入。

## 已移除内容

以下文档已不再保留：

- 早期 `phase0/phase1/phase2` 分阶段方案
- `Chassis` 导入清单一类的过程性迁移草稿
- `project_overview_and_target_architecture.md`
- `runtime_realignment_plan.md`
- `runtime_file_migration_map.md`

原因：

- 它们描述的是过渡阶段，不再代表当前仓库状态
- 当前仓库已经完成第一轮导入和主线收口
- 继续保留会让文档口径和代码状态脱节
