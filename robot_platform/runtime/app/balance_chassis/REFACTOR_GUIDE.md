# `balance_chassis` Runtime Refactor Guide

## 当前结论

`balance_chassis` 这一轮重构已经不再是“先设计一套新架构”，而是把现有控制链收成可执行、可验证、可对接的 runtime 主线。

当前主线可以压成：

`rc_data / ins_data -> chassis_cmd / chassis_observe -> chassis_state / leg_left / leg_right -> actuator_cmd`

其中：

- `rc_data` 是板级输入收口 topic
- `ins_data` / `chassis_cmd` 是正式输入边界
- `chassis_state` / `leg_left` / `leg_right` 是当前正式输出边界
- `chassis_observe` / `actuator_cmd` 仍属于过渡或内部执行边界

更完整的对接口径以 [platform_architecture.md](/home/xbd/workspace/codes/Astra_RM_Platform/robot_platform/docs/platform_architecture.md) 为准。

---

## 已完成

### 1. 构建链收口

- [x] 删除 `legacy_full` / `legacy_obj`
- [x] 当前公开主线只保留 `hw_elf` 与 `sitl`
- [x] `python3 -m robot_platform.tools.platform_cli.main build hw_elf` 可通过
- [x] `python3 -m robot_platform.tools.platform_cli.main build sitl` 可通过

### 2. 运行时总线主链立起来

- [x] `INS_task` 发布 `ins_data`
- [x] `remote_task` 订阅 `rc_data / ins_data / chassis_state / leg_left / leg_right`，发布 `chassis_cmd`
- [x] `observe_task` 订阅 `chassis_cmd`，发布 `chassis_observe`
- [x] `chassis_task` 订阅 `ins_data / chassis_cmd / chassis_observe`，发布 `chassis_state / leg_left / leg_right / actuator_cmd`
- [x] `motor_control_task` 改为订阅单一 `actuator_cmd`

### 3. legacy 全局状态显式依赖减少

- [x] `remote_task` 不再直接读 `rc_ctrl / INS / left / right / chassis_move`
- [x] `observe_task` 不再直接读 `INS / left / right / chassis_move`
- [x] `motor_control_task` 不再直接拼 `left/right/chassis_move`
- [x] `INS_task` 的 `INS` 运行态已收进文件内部
- [x] `chassis_task` 的 `chassis_move / right / left / PID` 运行态已收进文件内部

### 4. 输入注入层与控制层拆开

- [x] `rc_input_task` 已从 app 逻辑中移出，迁到 `runtime/bsp/devices/remote_control/rc_input_bridge.c`
- [x] app 目录中的重复 `remote_control.c/.h` 已删除
- [x] 遥控输入统一走 `runtime/bsp/devices/remote_control`

### 5. 执行层边界更清楚

- [x] 新增 `actuator_cmd`
- [x] `motor_control_task` 从“订阅多个 topic 再拼执行命令”改为“只订阅末端执行命令 topic”

---

## 当前边界

### 正式输入边界

- `ins_data`
- `chassis_cmd`

### 正式输出边界

- `chassis_state`
- `leg_left`
- `leg_right`

### 过渡或内部边界

- `rc_data`
- `chassis_observe`
- `actuator_cmd`

### 当前明确禁止的对接方式

- 直接依赖 `INS`
- 直接依赖 `chassis_move`
- 直接依赖 `left / right`
- 直接依赖 `rc_ctrl`

---

## 仍在进行

### A. `chassis_task` 仍然是控制大文件

虽然运行态已经收进文件内部，但 `chassis_task.c` 仍然同时承载：

- 反馈更新
- LQR 与补偿控制
- 跳跃状态机
- 离地检测
- 执行命令封装

这意味着当前的“总线边界”已经比之前清楚，但“控制内部职责拆分”还没完成。

### B. `chassis_observe` 仍是过渡 topic

它现在已经是 `observe_task` 唯一正式输出，不再回写 `chassis_move` 关键观测状态。  
但它还没有被升级为正式外部观测接口，所以不建议固化进 `sim/report/replay` 协议。

### C. 执行层仍保留少量 accessor

当前仍有少量窄 accessor 供执行层或观测层读取内部运行态，例如：

- `chassis_joint_motor_state()`
- `chassis_wheel_speed()`
- `chassis_set_joint_enable_flag()`

这些比 extern 全局状态已经好很多，但仍不是最终理想状态。

---

## 下一步优先级

### P0. 继续压缩 `chassis_task` 对外 accessor

目标：

- 能转成 topic 的继续转成 topic
- 执行层不要继续依赖控制层内部结构体地址
- 让 accessor 只保留确实无法立刻 topic 化的硬件执行接口

### P1. 拆 `chassis_task.c` 内部职责

建议拆分方向：

- `balance_controller.c`
- `jump_fsm.c`
- `ground_detect.c`
- `feedback_update.c`

要求：

- `chassis_task.c` 逐步退化成“订阅输入 -> 调用控制模块 -> 发布输出”的薄调度层

### P2. 明确 `actuator_cmd` 的长期命运

两种路线二选一：

1. 保持它为内部执行边界，只给 `motor_control_task` 使用
2. 升级它为统一执行输出接口，让硬件执行层和 SITL bridge 都接这一层

当前还没有最终定论，所以文档里仍把它定义为过渡 topic。

### P3. 统一 replay / sim / bridge 的边界口径

当前 sim 应优先对接：

- 输入：`ins_data`、`chassis_cmd`
- 输出：`chassis_state`、`leg_left`、`leg_right`

不要直接依赖：

- `rc_data`
- `chassis_observe`
- `actuator_cmd`

除非后续明确把其中某一项升级为正式边界。

---

## 不再作为当前执行面的旧计划

下面这些内容不再适合作为本文件的当前主计划：

- 回到 Keil 工程组织方式继续重排目录
- 先做一套新的 topic enum/ID 总线再说
- 把所有内容一次性抽成大量小模块后再验证
- 以 `legacy_full` 为主验证链

这些要么已经过时，要么会稀释当前最重要的交付目标。

---

## 当前交付标准

当下这一阶段的 `balance_chassis` runtime 重构，至少要满足：

- `hw_elf` 和 `sitl` 共用同一条控制主链
- `sim` 不需要直接读写 app 内部全局状态
- 输入注入留在 `bsp / 注入层`
- 控制逻辑留在 `app`
- 正式边界、过渡边界、内部边界有明确口径

如果不满足这几条，就还不能算交付完成。
