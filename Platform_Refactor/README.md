# Platform Refactor

`Platform_Refactor/` 是路线决策归档目录。

它的职责只有一个：解释这个项目为什么最后收敛成现在这条路线。

## 它主要回答什么问题

- 为什么 `robot_platform` 成为主开发入口
- 为什么 `Astra_RM2025_Balance` 只保留为历史基线
- 为什么当前验证主线是 `SITL + replay`
- 为什么 `physics_sim`、`osal`、大范围重构没有进入当前执行面

## 它不负责什么

- 不负责指导当前日常开发
- 不负责描述最新代码结构
- 不负责列执行清单或阶段任务

这些内容统一看：

- [../robot_platform/README.md](../robot_platform/README.md)
- [../robot_platform/docs/README.md](../robot_platform/docs/README.md)

## 当前保留文件

```text
Platform_Refactor/
  README.md
```

这里不再保留过程性路线拆解文档。
当前主线和执行文档统一看 `robot_platform/docs/`。

## 使用方式

如果你是在继续推进项目，实现代码、构建、验证链，优先看 `robot_platform/`。

如果你是在做方向对齐，想回答“这个项目到底是什么、为什么这么定”，再看这里。
