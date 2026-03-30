# cubemx_backend

当前阶段优先目标不是“导入现有项目”，而是先独立跑通 `CubeMX` 代码生成链。

## 第一阶段目标

先支持：

1. 输入一个现有 `.ioc`
2. 调用本机 `CubeMX` CLI
3. 生成到平台固定目录

## 本机 CLI 发现策略

后端优先按以下顺序寻找可执行文件：

1. 环境变量 `STM32CUBEMX_BIN`
2. `/home/xbd/.local/stm32cubemx/6.17.0/STM32CubeMX`
3. `PATH` 中的 `STM32CubeMX`
4. `PATH` 中的 `stm32cubemx`

## 当前仓库建议验证输入

- `/home/xbd/worspcae/code/references/legacy/Astra_RM2025_Balance_legacy/Chassis/CtrlBoard-H7_IMU.ioc`

## 当前状态

后端目录已建立，本机也已有可用 `CubeMX` CLI。

当前实测状态：

1. `STM32CubeMX 6.17.0` 可安装并以 `-q <script>` 启动
2. `project generate` 在当前 `Chassis` 工程上不稳定
3. `config load + generate code + exit` 已经对 `CtrlBoard-H7_IMU.ioc` 验证成功

因此第一版平台主线固定为：

- 使用旧 `.ioc` 作为硬件配置源
- 只用 CubeMX 生成 `generated code`
- 不依赖 CubeMX 生成最终完整项目工程
