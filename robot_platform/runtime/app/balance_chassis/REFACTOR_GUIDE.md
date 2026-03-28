# `balance_chassis` Runtime Refactor Guide

## 当前定位

`balance_chassis` 现在处于“app 目录语义已立住，但 runtime 分层还在回正”的阶段。

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

当前目录的长期定位仍是：

- `app_bringup/`
  - 线程创建和 app 入口
  - 当前文件：`freertos_app.c`、`task_registry.c`
- `app_io/`
  - app 级 topic/input/output 适配
  - 当前文件：`topic_contract.h`、`chassis_topics.*`、`remote_topics.*`
- `app_flow/`
  - 业务编排与运行态装配
  - 当前文件：`chassis_orchestration.*`、`remote_orchestration.*`、`remote_runtime.*`
- `app_config/`
  - 项目参数、topic 结构、业务常量、app 运行态结构
  - 当前文件：`robot_def.h`、`runtime_state.h`、`app_params.h`
- `legacy/`
  - 尚未拆完的 task shell 与兼容资产
  - 当前文件：`INS_task.*`、`chassis_task.*`、`remote_task.*`、`observe_task.*`

`runtime_service/`

- `actuator/`
  - 执行器运行时链路
  - 当前文件：`actuator_runtime.*`、`actuator_topics.*`、`motor_control_task.*`
- `sensing/`
  - 传感器输入整理
  - 当前为新建空层，等待迁入 `ins_runtime.*`
- `observe/`
  - 观测运行时组织
  - 当前为新建空层，等待迁入 `observe_*`
- `control_runtime/`
  - 控制运行态 glue
  - 当前为新建空层，等待迁入 `chassis_control_support.*`

当前平台目标结构是：

`generated + bsp + device + runtime_service + module + app + sim`

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

- [x] `control/` 已改为 `app_flow/`
- [x] `io/` 已改为 `app_io/`
- [x] `freertos_legacy.c` 已迁到 `app_bringup/freertos_app.c`
- [x] `robot_def.h` 已迁到 `app_config/robot_def.h`
- [x] 根目录 `*_task.c/.h` 已迁到 `legacy/`
- [x] `runtime/device/` 已创建首批空层
- [x] `runtime/runtime_service/` 已创建首批空层
- [x] actuator 线首批文件已迁入 `runtime_service/actuator/`

### 4. `app` 依赖面收窄

- [x] `app_io/chassis_topics.h` 不再直接依赖 `chassis_task.h`
- [x] `app_io/chassis_topics.h` 不再直接依赖 `INS_task.h`
- [x] `app_io/chassis_topics.h` 不再直接依赖 `VMC_calc.h`
- [x] `app_io/remote_topics.h` 只暴露 app 级 contract，不暴露 remote runtime 类型
- [x] `topic_contract.h` 已作为 app 级 topic 契约入口引入
- [x] `observe_task` 已开始拆成 `app_io/observe_topics.* + app_flow/observe_orchestration.* + task shell`
- [x] `INS_task` 已开始拆成 `app_io/ins_topics.* + app_flow/ins_runtime.* + task shell`
- [x] `motor_control_task` 已从 `app` 迁入 `runtime_service/actuator/`

### 5. `app_config` 开始承接运行态事实源

- [x] `INS_t` / `chassis_t` 已从 `legacy` 头抽到 `app_config/runtime_state.h`
- [x] `app_flow` 和 `VMC_calc.h` 不再直接依赖 `legacy/INS_task.h`、`legacy/chassis_task.h`
- [x] 坐标变换 helper 已从 `legacy/INS_task.c` 收回 `app_flow/ins_runtime.c`
- [x] 任务周期、线程栈、优先级、启动延时已开始收进 `app_config/app_params.h`

### 6. `app_bringup` 开始变成装配入口

- [x] `freertos_app.c` 已不再直接堆放所有 task wrapper
- [x] `task_registry.c` 已接管 balance_chassis 的线程注册

### 7. 总线主链已立起来

- [x] `INS_task` 发布 `ins_data`
- [x] `remote_task` 订阅 `rc_data / ins_data / chassis_state / leg_left / leg_right`，发布 `chassis_cmd`
- [x] `observe_task` 订阅 `chassis_cmd / actuator_feedback`，发布 `chassis_observe`
- [x] `chassis_task` 订阅 `ins_data / chassis_cmd / chassis_observe / actuator_feedback`，发布 `chassis_state / leg_left / leg_right / actuator_cmd`
- [x] `motor_control_task` 订阅 `actuator_cmd`，发布 `actuator_feedback`

---

## 仍未完成

### 1. `runtime_service` 还没完全接住 sensing / observe / control_runtime

目前只有 actuator 线开始从 `app` 拔出。

还没迁的重点是：

- `app_flow/ins_runtime.*`
- `app_io/ins_topics.*`
- `app_flow/observe_orchestration.*`
- `app_io/observe_topics.*`
- `app_flow/chassis_control_support.*`

这说明 app 仍然吃进了一部分 sensing / observe / control runtime glue。

### 2. `legacy/` 只是位置收口，还不是完成拆分

现在 task 文件已经迁入 `legacy/`，但这还只是承认迁移期，不等于拆分完成。

下一步仍要继续把这些文件压成真正的 shell。

### 3. `chassis_task.c` 仍然偏重

虽然它已经变成“拉输入 -> 调控制 -> 发输出”的主干，但里面仍保留：

- app bundle 到 legacy runtime 的装配
- 控制周期内的部分 legacy 状态更新
- 输出封装

它还没彻底退化成薄调度壳。

### 4. 根目录 task shell 只完成了局部收口

当前完成度：

- `legacy/remote_task.c` 已基本是 shell
- `legacy/observe_task.c` 已基本是 shell
- `legacy/INS_task.c` 已基本是 shell
- `freertos_app.c` 已基本是入口壳
- `legacy/observe_task.c` 里的未使用卡尔曼残留已清掉

还没完成：

- `legacy/chassis_task.c`
- `legacy` 头文件的进一步瘦身
- `app_flow` 内部更细的业务编排拆分

---

## 下一步顺序

### P0. 先把 runtime_service 其余三条线立起来

优先顺序：

1. `sensing`
2. `observe`
3. `control_runtime`

目标是先把 `app` 吃进去的运行时 glue 拔出去。

### P1. 再继续把 task 壳层和 legacy 本体拆开

优先顺序：

1. `chassis_task.c`
2. `observe_task.c`
3. `INS_task.c`

目标是把 task shell、app logic、app io 拆开，让根目录 task 只剩调度骨架。

### P2. 继续拆薄 `legacy/` 里的 task

重点是把“任务壳 / app_flow / app_io / legacy runtime”再切清楚，而不是停在目录重命名。

### P3. 再收 `app_io` 的 contract

重点是明确：

- 哪些 topic 是长期正式契约
- 哪些 topic 只是内部执行桥
- 哪些 struct 还能继续出现在 app 装配层

---

## 当前交付标准

当前阶段要算完成，至少需要满足：

- `hw_elf` 和 `sitl` 持续可构建
- app 目录语义明确是 `app_bringup / app_io / app_flow / app_config / legacy`
- `runtime_service` 至少具备 `actuator / sensing / observe / control_runtime` 四条服务线
- `sim` 不需要依赖 app 内部全局状态
- `app_io` 不再在边界头文件上暴露 legacy task 和算法类型
- `app_flow` 和算法模块不再反向依赖 `legacy` 头
- task 文件逐步退化成装配壳，而不是继续承担平台细节
