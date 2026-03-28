# robot_platform docs

这里仅保留当前还能直接指导平台开发、验证和环境搭建的文档。

## 当前有效文档

1. [platform_architecture.md](./platform_architecture.md)
   当前平台项目的统一架构说明，定义项目是什么、目标架构是什么、当前边界和演进目标是什么。
2. [project_roles_and_scope.md](./project_roles_and_scope.md)
   定义项目分工、执行主线、优先级和接口决策边界。
3. [generated_import_rules.md](./generated_import_rules.md)
   约束 `runtime/generated/` 的边界，避免把业务逻辑重新塞回生成目录。
4. [wsl_environment_setup.md](./wsl_environment_setup.md)
   记录当前仓库在 WSL 下跑通 `generate` / `build` 所需的环境前提。
5. [sim_architecture_research.md](./sim_architecture_research.md)
   调研当前 `sim/SITL` 架构方向，说明为什么采用平台公共 `sim core` 与项目适配层的结构。

## 已移除内容

以下文档已不再保留：

- 早期 `phase0/phase1/phase2` 分阶段方案
- `Chassis` 导入清单一类的过程性迁移草稿

原因：

- 它们描述的是过渡阶段，不再代表当前仓库状态
- 当前仓库已经完成第一轮导入和主线收口
- 继续保留会让文档口径和代码状态脱节
