# SITL Roadmap

这个文档不再记录早期方案比较，只保留当前仍成立的路线判断。

## 当前结论

平台主线已经收敛为：

1. `CubeMX` 生成链
2. `CMake + GCC` 硬件构建链
3. `Linux + FreeRTOS POSIX port` 的 SITL 构建链
4. 基于 SITL 的 replay 验证

当前明确不作为近期目标：

- 自建 `osal`
- `physics_sim`
- 额外的上位机通信体系扩展

## 为什么主验证路线是 SITL

当前项目最需要的是：

- 在不依赖真板的情况下运行完整控制链
- 尽量保留现有 task 调度与运行时行为
- 给后续 replay 留统一承载环境

因此当前采用：

- 固件编译为 Linux 进程
- FreeRTOS 使用 POSIX/Linux 端口
- BSP 中与硬件强绑定的部分由 SITL 桩替代
- Python bridge 负责向 SITL 注入虚拟输入并收集输出

## 当前仓库已经具备的基础

- `cmake/toolchains/linux-gcc.cmake`
- `third_party/freertos_port_linux/`
- `runtime/bsp/sitl/`
- `sim/bridge/sim_bridge.py`
- `build sitl` CLI 入口

这说明当前问题已经不是“要不要做 SITL”，而是“如何把 SITL 变成可重复验证链”。

## 下一阶段只保留 3 个工作包

### 1. SITL 稳定化

目标：

- 保证 `balance_chassis_sitl` 稳定构建
- 收紧 BSP SITL 桩和 bridge 的接口
- 让 CLI 与真实 SITL 入口一致

验收：

- `python3 -m robot_platform.tools.platform_cli.main build sitl` 可稳定通过
- SITL 进程可启动并进入当前任务调度链

### 2. Replay

目标：

- 板上记录关键输入输出
- 在 SITL 中按时间回放
- 给重构提供行为对比基线

验收：

- 至少有一条可重复运行的日志回放链
- 能输出基础的 pass/fail 对比结果

### 3. Runtime 收敛

目标：

- 减少 `module` 对 `main.h`、任务头、板级实现的直连
- 继续收敛 `message_center` / 全局变量 / 历史 topic 试验思路
- 为 replay 和后续测试留出更稳定的边界

验收：

- 关键控制模块可在不直接引用板级头文件的前提下参与构建
- `hw` 与 `sitl` 使用同一条主控制链

## 近期不做的事

以下事项确认延后，不作为当前排期：

- `physics_sim` 和场景库
- 高保真动力学建模
- 大范围算法 step 化
- 新消息总线方案落地

这些方向不是没有价值，而是当前资源下会稀释主线，先不进入执行面。

## 当前建议的推进顺序

1. 继续保持 `hw_elf` 和 `sitl` 双构建链稳定
2. 先完成一条最小 replay 闭环
3. 再做运行时边界收敛
4. 最后补 `flash / debug / test` 的统一 CLI
