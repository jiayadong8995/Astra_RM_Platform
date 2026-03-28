# WSL Environment Setup

最后验证日期：2026-03-28

本文档记录当前仓库在 WSL 下跑通 `robot_platform` 最小硬件链路所需的环境、安装步骤和当前实测结论。

## 当前实测结果

已验证通过：

- `java`
- `ninja`
- `arm-none-eabi-gcc`
- `python3 -m robot_platform.tools.platform_cli.main build`

当前仍有条件性阻塞：

- 在无显示能力的自动化会话中，`python3 -m robot_platform.tools.platform_cli.main generate`

原因不是 `.ioc` 路径或 CubeMX 二进制缺失，而是 `STM32CubeMX 6.17.0` 在纯 headless 会话里，即使带 `-q <script>` 仍会初始化 Swing 主窗口，最终抛出 `java.awt.HeadlessException`。

换句话说：

- 构建链已经通
- CubeMX 安装已经通
- CubeMX CLI 入口已经接上
- 在带 `DISPLAY` 或 `WAYLAND_DISPLAY` 的 WSLg 终端中，应优先直接运行 `generate`
- 只有在纯无显示会话里，才需要额外的图形显示环境，例如 `WSLg`、外部 `X Server`，或后续补装 `Xvfb`

## 系统依赖

建议安装：

```bash
sudo apt-get update
sudo apt-get install -y default-jre ninja-build gcc-arm-none-eabi
```

当前实测版本：

```text
java: openjdk 21.0.10
ninja: 1.11.1
arm-none-eabi-gcc: 13.2.1
python3: 3.12.3
cmake: 3.28.3
```

## CubeMX 安装包

当前使用的离线安装包路径：

```text
/home/xbd/workspace/stm32cubemx-lin-v6-17-0 (1).zip
```

解压后会得到：

```text
/tmp/stm32cubemx_617/SetupSTM32CubeMX-6.17.0
```

## CubeMX 安装位置

在当前仓库内，已安装到：

```text
/home/xbd/workspace/codes/Astra_RM_Platform/.local_tools/stm32cubemx/6.17.0
```

主可执行文件：

```text
/home/xbd/workspace/codes/Astra_RM_Platform/.local_tools/stm32cubemx/6.17.0/STM32CubeMX
```

之所以安装到仓库内，而不是 `~/.local`，是因为当前自动化运行环境只保证仓库目录可写。

## CubeMX 安装命令

如果本机也使用相同的离线安装包，可以直接用控制台模式安装。

1. 解压安装包：

```bash
rm -rf /tmp/stm32cubemx_617
mkdir -p /tmp/stm32cubemx_617
unzip -q "/home/xbd/workspace/stm32cubemx-lin-v6-17-0 (1).zip" -d /tmp/stm32cubemx_617
```

2. 生成安装模板：

```bash
DISPLAY= /tmp/stm32cubemx_617/SetupSTM32CubeMX-6.17.0 -options-template /tmp/stm32cubemx_617/options-template.txt
```

3. 写入目标安装路径：

```bash
cp /tmp/stm32cubemx_617/options-template.txt /tmp/stm32cubemx_617/options-repo.txt
sed -i 's#^INSTALL_PATH=.*#INSTALL_PATH=/home/xbd/workspace/codes/Astra_RM_Platform/.local_tools/stm32cubemx/6.17.0#' /tmp/stm32cubemx_617/options-repo.txt
mkdir -p /home/xbd/workspace/codes/Astra_RM_Platform/.local_tools/stm32cubemx/6.17.0
```

4. 执行控制台安装：

```bash
DISPLAY= /tmp/stm32cubemx_617/SetupSTM32CubeMX-6.17.0 -console -options /tmp/stm32cubemx_617/options-repo.txt
```

## 当前仓库内的 CubeMX 查找逻辑

当前后端已经支持以下查找顺序：

1. 环境变量 `STM32CUBEMX_BIN`
2. 仓库内 `.local_tools/stm32cubemx/6.17.0/STM32CubeMX`
3. `/home/xbd/.local/stm32cubemx/6.17.0/STM32CubeMX`
4. `PATH` 中的 `STM32CubeMX`
5. `PATH` 中的 `stm32cubemx`

## 当前 `.ioc` 输入位置

当前 `generate` 命令读取的是：

```text
/home/xbd/workspace/codes/Astra_RM_Platform/Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc
```

## 当前命令

构建命令：

```bash
python3 -m robot_platform.tools.platform_cli.main build
```

当前实测通过，产物位于：

```text
/home/xbd/workspace/codes/Astra_RM_Platform/build/robot_platform_ninja/balance_chassis_hw_seed.elf
```

生成命令：

```bash
python3 -m robot_platform.tools.platform_cli.main generate
```

当前会调用：

- `config load "<ioc>"`
- `generate code "<output dir>"`
- `exit`

脚本文件位于：

```text
/tmp/robot_platform_codegen/generate_from_ioc.mxs
```

## generate 当前阻塞与建议

当前报错特征：

```text
java.awt.HeadlessException
```

这说明在当前 WSL 会话中，`STM32CubeMX -q` 仍然依赖图形显示能力。

建议优先级：

1. 如果当前机器支持 `WSLg`，在有图形能力的 WSL 终端里重新执行 `generate`
2. 如果没有 `WSLg`，安装并验证一个可用的 `X Server`
3. 或者后续补装 `Xvfb`，用虚拟显示驱动 CubeMX CLI

## 当前仓库内做过的适配

为适配当前 WSL 环境，仓库已做以下调整：

- `platform_cli generate` 已改为读取 `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc`
- `platform_cli` 中 `sim` 改成按需导入，避免影响 `generate/build`
- `cubemx_backend` 已支持查找仓库内安装的 CubeMX
- `cubemx_backend` 会把 CubeMX 的 `HOME` 和 `user.home` 指向仓库内可写目录 `.cache/stm32_user_home`
- `cubemx_backend` 在检测到 `DISPLAY` 或 `WAYLAND_DISPLAY` 时，不再强制 headless 模式
