# robot_platform docs

这里只保留当前还能直接指导实现、验证和环境搭建的文档。

## 当前基线

1. [platform_architecture.md](./platform_architecture.md)
   平台总架构说明，回答项目边界、主层次和当前执行主线。
2. [runtime_architecture_blueprint.md](./runtime_architecture_blueprint.md)
   运行时正式架构基线，固定 `generated / bsp / device / control / app / sim`。
3. [runtime_contracts_draft.md](./runtime_contracts_draft.md)
   主控制链正式契约，定义 `DeviceInput / RobotState / RobotIntent / ActuatorCommand / DeviceCommand / DeviceFeedback`。
4. [runtime_capability_boundaries.md](./runtime_capability_boundaries.md)
   运行时能力边界表，说明哪些能力归属 `device / control / app / sim`。
5. [runtime_communication_matrix.md](./runtime_communication_matrix.md)
   通信矩阵，固定主控制链和广播链的通信方式。
6. [runtime_architecture_task_list.md](./runtime_architecture_task_list.md)
   当前剩余任务和阶段交付标准。
7. [project_roles_and_scope.md](./project_roles_and_scope.md)
   项目职责和当前阶段边界。
8. [generated_import_rules.md](./generated_import_rules.md)
   生成目录边界约束。
9. [wsl_environment_setup.md](./wsl_environment_setup.md)
   环境搭建说明。

## 参考文档

1. [sim_architecture_research.md](./sim_architecture_research.md)
   `sim` 方向的参考研究，不作为当前唯一实现口径。
2. [ros_robot_algorithm_placement_research.md](./ros_robot_algorithm_placement_research.md)
   ROS 与高质量机器人项目的算法归属调研，用于校验 `device / control / app` 的职责边界。

## 已收口内容

以下内容已删除或降级，不再作为当前实现依据：

- `app_layer_architecture_research.md`
- `runtime_migration_stages.md`
- `project_overview_and_target_architecture.md`
- `runtime_realignment_plan.md`
- `runtime_file_migration_map.md`
- 早期 `phase0/phase1/phase2` 分阶段方案

原因很简单：

- 当前主架构已经定型
- 主数据对象已经统一到 contracts
- 继续保留这些过程文档只会制造 review 噪音
