# `balance_chassis` Runtime Refactor Guide

## 当前定位

`balance_chassis` 现在处于“legacy task 已迁入平台，但 app 还没完全收成装配层”的阶段。

这一层最终只应承担 4 件事：

1. 任务调度
2. 项目业务装配
3. app 边界输入输出适配
4. 模式与生命周期管理

它不应继续承担：

- 算法实现
- 板级设备细节
- 大量 legacy 结构体转换
- 消息总线实现细节

---

## 当前目录语义

当前目录已经开始向目标结构收口：

- `app_main/`
  - 线程创建和 app 入口
  - 当前文件：`freertos_app.c`
- `app_io/`
  - app 级 topic/input/output 适配
  - 当前文件：`topic_contract.h`、`chassis_topics.*`、`remote_topics.*`
- `app_logic/`
  - 业务编排 helper
  - 当前文件：`chassis_runtime_helpers.*`、`remote_runtime.*`
- `app_config/`
  - 项目参数、topic 结构、业务常量
  - 当前文件：`robot_def.h`
- 根目录 `*_task.c`
  - 仍属于 legacy task 本体
  - 当前还是过渡区，后续应逐步迁入 `legacy/`

目标结构仍然是：

`app_main + app_io + app_logic + app_config + legacy`

---

## 当前主链

当前运行时主链是：

`rc_data / ins_data -> chassis_cmd / chassis_observe -> chassis_state / leg_left / leg_right -> actuator_cmd`

执行反馈链是：

`motor_control_task / device feedback -> actuator_feedback -> observe_task / chassis_task`

当前边界口径：

- 正式输入：`ins_data`、`chassis_cmd`
- 正式输出：`chassis_state`、`leg_left`、`leg_right`
- 过渡或内部边界：`rc_data`、`chassis_observe`、`actuator_cmd`、`actuator_feedback`

---

## 已完成

### 1. 主线收口

- [x] 删除 `legacy_full` / `legacy_obj`
- [x] 当前公开主线只保留 `hw_elf` 与 `sitl`
- [x] `python3 -m robot_platform.tools.platform_cli.main build hw_elf` 可通过
- [x] `python3 -m robot_platform.tools.platform_cli.main build sitl` 可通过

### 2. 行为问题修复

- [x] `remote_task` 去掉额外 `osDelay(50)`
- [x] `freertos_app` 在 `SITL_BUILD` 下也创建 `REMOTE_TASK` 与 `RC_INPUT_TASK`
- [x] `INS_task.h` / `ins_task.h` 兼容双头文件已收掉，只保留 `INS_task.h`

### 3. 目录语义迁移

- [x] `control/` 已改为 `app_logic/`
- [x] `io/` 已改为 `app_io/`
- [x] `freertos_legacy.c` 已迁到 `app_main/freertos_app.c`
- [x] `robot_def.h` 已迁到 `app_config/robot_def.h`

### 4. `app_io` 依赖面收窄

- [x] `app_io/chassis_topics.h` 不再直接依赖 `chassis_task.h`
- [x] `app_io/chassis_topics.h` 不再直接依赖 `INS_task.h`
- [x] `app_io/chassis_topics.h` 不再直接依赖 `VMC_calc.h`
- [x] `app_io/remote_topics.h` 只暴露 app 级 contract，不暴露 remote runtime 类型
- [x] `topic_contract.h` 已作为 app 级 topic 契约入口引入

### 5. 总线主链已立起来

- [x] `INS_task` 发布 `ins_data`
- [x] `remote_task` 订阅 `rc_data / ins_data / chassis_state / leg_left / leg_right`，发布 `chassis_cmd`
- [x] `observe_task` 订阅 `chassis_cmd / actuator_feedback`，发布 `chassis_observe`
- [x] `chassis_task` 订阅 `ins_data / chassis_cmd / chassis_observe / actuator_feedback`，发布 `chassis_state / leg_left / leg_right / actuator_cmd`
- [x] `motor_control_task` 订阅 `actuator_cmd`，发布 `actuator_feedback`

---

## 仍未完成

### 1. `app_io` 还不是纯 adapter

虽然 `app_io` 已经不再在头文件上暴露 legacy task 和算法类型，但 `chassis_task.c` 里仍存在 app-level bundle 到 legacy runtime 的映射逻辑。

这说明：

- 旧的 `topic -> legacy runtime` 胶水没有消失
- 只是从 `app_io` 头文件层面收回到了 task shell 内部

下一步应继续把这部分抽成更明确的 app 装配逻辑，而不是继续堆在 `chassis_task.c`。

### 2. 根目录 `*_task.c` 还未迁到 `legacy/`

现在的 task 文件已经比以前更像 shell，但还没有完成目录级隔离。

下一步需要逐步收成：

- `legacy/INS_task.c`
- `legacy/chassis_task.c`
- `legacy/remote_task.c`
- `legacy/observe_task.c`
- `legacy/motor_control_task.c`

### 3. `chassis_task.c` 仍然偏重

虽然它已经变成“拉输入 -> 调控制 -> 发输出”的主干，但里面仍保留：

- app bundle 到 legacy runtime 的装配
- 控制周期内的部分 legacy 状态更新
- 输出封装

它还没彻底退化成薄调度壳。

---

## 下一步顺序

### P0. 继续把 task 壳层和 legacy 本体拆开

优先顺序：

1. `chassis_task.c`
2. `observe_task.c`
3. `INS_task.c`
4. `motor_control_task.c`

目标是把 task shell、app logic、app io 拆开，让根目录 task 只剩调度骨架。

### P1. 把根目录 task 迁入 `legacy/`

当 task shell 足够薄之后，再做目录级迁移，避免先搬目录再继续在里面堆逻辑。

### P2. 再收 `app_io` 的 contract

重点是明确：

- 哪些 topic 是长期正式契约
- 哪些 topic 只是内部执行桥
- 哪些 struct 还能继续出现在 app 装配层

---

## 当前交付标准

当前阶段要算完成，至少需要满足：

- `hw_elf` 和 `sitl` 持续可构建
- app 目录语义明确是 `app_main / app_io / app_logic / app_config`
- `sim` 不需要依赖 app 内部全局状态
- `app_io` 不再在边界头文件上暴露 legacy task 和算法类型
- task 文件逐步退化成装配壳，而不是继续承担平台细节
