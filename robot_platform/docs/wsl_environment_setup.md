# WSL Environment Setup

本文档描述 `robot_platform` 在 WSL 环境下运行所需的前提、验证方式和已知限制。

它的目标是回答平台层面的问题：

- 需要哪些工具
- 平台如何发现这些工具
- 在 WSL 下哪些命令应当可以工作
- 当前有哪些环境限制

它不记录个人机器路径、一次性安装过程或本地临时文件位置。

## 1. 适用范围

本文档适用于在 WSL 中执行以下平台能力：

- `generate`
- `build hw_elf`
- `build sitl`

## 2. 环境前提

在 WSL 中使用 `robot_platform`，至少需要以下工具可用：

- `python3`
- `cmake`
- `ninja`
- `arm-none-eabi-gcc`
- `java`
- `STM32CubeMX`

说明：

- `python3` 用于运行平台 CLI
- `cmake`、`ninja`、`arm-none-eabi-gcc` 用于硬件构建
- `java` 和 `STM32CubeMX` 用于代码生成

## 3. 平台如何发现工具

### 3.1 `STM32CubeMX`

当前后端按以下顺序查找 `STM32CubeMX`：

1. 环境变量 `STM32CUBEMX_BIN`
2. 仓库内 `.local_tools/stm32cubemx/6.17.0/STM32CubeMX`
3. 用户本地默认安装路径
4. `PATH` 中的 `STM32CubeMX`
5. `PATH` 中的 `stm32cubemx`

建议：

- 在团队或 CI 环境中，优先显式设置 `STM32CUBEMX_BIN`
- 不要让平台文档依赖某一台机器的私有安装路径

### 3.2 可写目录

当前 `CubeMX` 后端会使用仓库内可写缓存目录保存运行期配置：

- `.cache/stm32_user_home`

这属于平台实现细节，使用者只需要保证仓库目录可写。

## 4. 当前命令

### 4.1 代码生成

```bash
python3 -m robot_platform.tools.platform_cli.main generate
```

当前行为：

- 读取 `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc`
- 调用 `STM32CubeMX` CLI
- 输出到 `robot_platform/runtime/generated/stm32h7_ctrl_board_raw`

### 4.2 硬件构建

```bash
python3 -m robot_platform.tools.platform_cli.main build hw_elf
```

### 4.3 SITL 构建

```bash
python3 -m robot_platform.tools.platform_cli.main build sitl
```

## 5. 推荐验证顺序

在新的 WSL 环境中，建议按下面顺序验证：

1. 先验证工具存在：
   - `python3 --version`
   - `cmake --version`
   - `ninja --version`
   - `arm-none-eabi-gcc --version`
   - `java -version`
2. 再验证平台构建：
   - `python3 -m robot_platform.tools.platform_cli.main build hw_elf`
   - `python3 -m robot_platform.tools.platform_cli.main build sitl`
3. 最后验证生成：
   - `python3 -m robot_platform.tools.platform_cli.main generate`

这样做的原因是：

- 构建链通常比 `CubeMX` 图形依赖更稳定
- 先确认构建链无误，再排查 `generate` 的环境问题更容易

## 6. WSL 下的已知限制

当前最主要的限制在 `generate`：

- `STM32CubeMX` 在部分 WSL 会话中仍依赖图形显示能力
- 在纯无显示环境中，可能出现 `java.awt.HeadlessException`

这意味着：

- `build` 类命令通常可以在普通 WSL 终端或自动化环境中直接运行
- `generate` 更适合在具备图形显示能力的 WSL 环境中执行，例如 `WSLg` 或可用的 X Server

## 7. 平台层面的建议

为了让 WSL 文档保持平台化质量，团队应当遵守以下原则：

- 文档只描述前提、规则、验证步骤和限制
- 不写个人机器专属路径
- 不写一次性安装过程的临时命令
- 不把本地排障记录直接当成平台文档

如果后续需要保留详细安装步骤，应另建：

- `local_setup/`
- 或团队内部运维文档

而不是继续堆在平台主文档里。

## 8. 当前结论

对 `robot_platform` 而言，WSL 环境下的结论可以压缩成两点：

1. `build` 是当前更稳定的能力，应优先验证
2. `generate` 可用，但仍受 `STM32CubeMX` 图形依赖约束
