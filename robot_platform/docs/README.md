# robot_platform docs

这里仅保留当前还能直接指导开发和环境搭建的文档。

## 当前有效文档

1. [generated_import_rules.md](./generated_import_rules.md)
   约束 `runtime/generated/` 的边界，避免把业务逻辑重新塞回生成目录。
2. [wsl_environment_setup.md](./wsl_environment_setup.md)
   记录当前仓库在 WSL 下跑通 `generate` / `build` 所需的环境前提。

## 已移除内容

以下文档已不再保留：

- 早期 `phase0/phase1/phase2` 分阶段方案
- `Chassis` 导入清单一类的过程性迁移草稿

原因：

- 它们描述的是过渡阶段，不再代表当前仓库状态
- 当前仓库已经完成第一轮导入和主线收口
- 继续保留会让文档口径和代码状态脱节
