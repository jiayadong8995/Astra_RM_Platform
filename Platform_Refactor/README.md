# Platform Refactor

这个目录现在只保留一类内容：

- 为什么平台最终收敛到当前路线
- 当前仍有参考价值的路线判断

它不再承担“正在执行的开发文档”职责。真正指导当前开发的文档，统一看 [robot_platform/docs/README.md](../robot_platform/docs/README.md)。

## 当前保留文件

```text
Platform_Refactor/
  README.md
  05_SITL_Roadmap.md
```

## 收口原则

以下内容已经从这里移除：

- 早期架构草图
- 迁移批次拆分稿
- 市场调研长文

原因：

- 这些文档描述的是立项和方案比较阶段
- 当前仓库已经完成第一轮主线收口
- 继续保留会把历史判断误读成当前要求

## 当前建议

如果只是继续开发平台，优先看：

1. [../robot_platform/README.md](../robot_platform/README.md)
2. [../robot_platform/docs/README.md](../robot_platform/docs/README.md)

如果要回顾为什么最后选了现在这条验证路线，再看：

1. [05_SITL_Roadmap.md](./05_SITL_Roadmap.md)
