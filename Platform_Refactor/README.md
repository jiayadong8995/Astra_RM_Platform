# Astra Platform Refactor

这个目录用于承载 `Astra_RM2025_Balance` 从 `CubeMX + Keil` 历史工程向“可生成、可构建、可回放、可仿真”的统一平台迁移方案。

当前阶段只做两件事：

1. 给现有仓库建立一套能落地的改造方案
2. 在不破坏现有 `Chassis/`、`Gimbal/`、`Framework/` 的前提下，搭出第一版平台骨架

不做的事：

1. 不直接改写现有控制算法
2. 不在这一轮里重做全部 CubeMX 工程
3. 不把 `Keil` 继续作为主入口

## 目录说明

```text
Platform_Refactor/
  README.md
  01_Architecture.md
  02_Migration_Map.md
  03_MVP_Roadmap.md
  skeleton/
    robot_platform/
      CMakeLists.txt
      tools/
        platform_cli/README.md
        schema/
          board.schema.yaml
          project.schema.yaml
      runtime/
        README.md
      projects/
        balance_chassis/
          board.yaml
          project.yaml
```

## 核心判断

基于当前仓库实际情况，第一版平台只建议先覆盖 `Chassis`：

- `Chassis` 已经初步有 `Bsp / Algorithm / Controller / APP` 层次
- `Chassis` 已经在尝试引入 message bus 和重构
- `Gimbal` 仍是典型旧 RM 风格，适合第二阶段再迁

## 第一版目标

第一版平台 MVP 只要求打通以下链路：

1. `project.yaml + board.yaml`
2. 生成器后端
3. `runtime/generated`
4. `CMake + arm-none-eabi-gcc`
5. `flash/debug`
6. `replay`
7. 最小 `physics_sim`

## 建议阅读顺序

1. [01_Architecture.md](./01_Architecture.md)
2. [02_Migration_Map.md](./02_Migration_Map.md)
3. [03_MVP_Roadmap.md](./03_MVP_Roadmap.md)

