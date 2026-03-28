# robot_platform

`robot_platform` 是整个仓库的当前主工程，也是平台架构定义与落地的主执行面。

它负责承接：

1. `CubeMX` 生成入口
2. `GCC/CMake` 构建入口
3. `generated / bsp / module / app` 运行时目录
4. 当前 `SITL` 验证主线
5. 新消息总线体系落地

## 目录

```text
robot_platform/
  CMakeLists.txt
  cmake/
  docs/
  projects/
  runtime/
  sim/
  tools/
```

## 从哪里开始

1. 先读 [../README.md](../README.md)
2. 再读 [docs/README.md](./docs/README.md)
3. 如需看架构说明，读 [docs/platform_architecture.md](./docs/platform_architecture.md)
4. 如需看分工和边界，读 [docs/project_roles_and_scope.md](./docs/project_roles_and_scope.md)
5. 如需看环境细节，读 [docs/wsl_environment_setup.md](./docs/wsl_environment_setup.md)
6. 如需看路线背景，再读 [../Platform_Refactor/README.md](../Platform_Refactor/README.md)

## 当前可用命令

```bash
python3 -m robot_platform.tools.platform_cli.main generate
python3 -m robot_platform.tools.platform_cli.main build hw_elf
python3 -m robot_platform.tools.platform_cli.main build sitl
```

## 说明

这个 `README` 只负责导航。

架构定义、角色分工、环境约束和平台边界，统一放在 `docs/` 中维护。
