# 03 MVP Roadmap

## 1. 里程碑定义

第一版不要做“大而全平台”，只做当前仓库的最小可用平台。

## M1 工具链壳子

目标：

- 平台目录建立
- 样板项目配置建立
- schema 初版建立
- CMake 工程骨架建立

验收：

- 平台仓结构稳定
- 能读到 `project.yaml` 和 `board.yaml`

## M2 生成与构建打通

目标：

- 生成后端 MVP
- `runtime/generated` 输出目录
- arm-none-eabi-gcc 构建
- elf/bin/map 输出

验收：

- 能从配置驱动生成目录
- 能编译出固件目标

## M3 板上最小运行

目标：

- 把 `Chassis` 的最小工程迁到平台骨架
- flash/debug 命令统一

验收：

- 板上能启动
- 基本外设初始化正常

## M4 运行时分层

目标：

- `bsp` 接管关键设备
- `module` 抽出核心控制链
- `app` 做任务装配

验收：

- 至少一条控制链不直接依赖 HAL

## M5 replay

目标：

- 统一日志格式
- 日志读取器
- 基线对比

验收：

- 至少一条真实日志可回放
- 有 pass/fail 报告

## M6 physics sim

目标：

- 简化动力学
- 控制闭环
- 三个基础场景

验收：

- 3 个场景自动运行并输出报告

## 2. 首轮任务拆解

### 第 1 轮

- 建 `Platform_Refactor/` 方案目录
- 固化迁移边界
- 建最小平台 skeleton

### 第 2 轮

- 补 `CMake toolchain`
- 补 CLI 协议
- 明确 `generated` 导入流程

### 第 3 轮

- 把 `Chassis` 的文件按新层次做一版映射迁移
- 不追求马上 clean refactor

### 第 4 轮

- replay 日志格式定稿
- 从现有项目抽一条数据链

## 3. 风险控制

### 风险 1

一开始就想把 `Chassis + Gimbal + Framework` 一起统一。

处理：

- 第一版只做 `Chassis`

### 风险 2

backend 过重，想把业务模板也塞进生成器。

处理：

- backend 只负责硬件生成

### 风险 3

目录改了，但依赖关系没改，最后只是“换皮”。

处理：

- 每轮迁移都要检查边界是否真的被切开

### 风险 4

过早追求高保真仿真。

处理：

- 先 replay
- 后 sim
- sim 先做最小闭环

## 4. 下一步建议

如果这个方向认可，下一轮建议直接进入“骨架实现”：

1. 在仓库里创建真实的 `robot_platform/` 目录
2. 增加最小 `CMakeLists.txt`
3. 增加 `project.yaml` / `board.yaml`
4. 增加 CLI 协议约定
5. 为 `Chassis` 做第一版文件迁移清单

