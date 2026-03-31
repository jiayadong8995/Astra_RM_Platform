# Balance Chassis Parameter Partition Refactor

## 背景

当前 `balance_chassis` 的参数体系主要集中在
[runtime/control/internal/balance_params.h](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/internal/balance_params.h)。

这个头文件同时承载了多类语义完全不同的参数：

- 机器人本体物理定义
- 控制器调参与阈值
- 遥控/操作映射
- 执行器极限与安全边界

此外，
[runtime/app/balance_chassis/app_config/robot_def.h](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/app/balance_chassis/app_config/robot_def.h)
目前只是一个空壳转发头，并没有形成真正的 robot-specific configuration boundary。

这会带来几个直接问题：

- `robot definition` 和 `controller internals` 被混在一起
- SITL / host runtime / hardware runtime 都难以共享清晰边界
- 上层 app 很容易直接 include 控制器内部参数头
- 后续想替换控制器、替换输入映射、替换机器人形态时，改动面会不必要地扩散

## 目标

将现有参数按职责拆分为独立边界，使 `SITL = 主机上运行真实 runtime，上层逻辑尽量真，底层适配可替换` 这条原则可以在代码结构上成立。

重构后参数体系至少分为以下 3 层：

1. `robot physical definition`
2. `control tuning params`
3. `operator / remote mapping params`

建议再单独补一层：

4. `actuator / safety limits`

## 设计原则

### 1. 机器人定义不依赖控制器

几何尺寸、轮径、关节零偏、腿长边界等参数，属于机器人本体定义，不应存放在 `control/internal/`。

### 2. 控制器参数不伪装成 robot config

PID、LQR、跌倒阈值、恢复阈值、重力补偿等属于控制策略配置，不应通过 `robot_def.h` 暗中暴露给 app。

### 3. 操作映射单独管理

遥控器通道到速度、转向、腿长目标的映射，应被视为 operator interface 配置，而不是控制器内部常量。

### 4. 安全边界单独管理

扭矩、电流、速度、腿长、功率等限制应集中到安全/执行器边界，而不是散落在控制器或 app 里。

### 5. 先拆语义，再拆实现

第一阶段先完成头文件和 include 边界迁移，不修改参数值与运行行为。确认测试稳定后，再进一步演进为结构体配置或项目 profile。

## 目标目录结构

建议新增如下头文件：

```text
robot_platform/runtime/app/balance_chassis/app_config/
  robot_physical_params.h
  remote_mapping_params.h

robot_platform/runtime/control/config/
  balance_control_params.h
  actuator_limits.h
```

## 参数归属建议

### A. robot physical definition

建议放入：
[runtime/app/balance_chassis/app_config/robot_physical_params.h](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/app/balance_chassis/app_config/robot_physical_params.h)

建议迁移的参数：

- `LINK_L1`
- `LINK_L2`
- `LINK_L3`
- `LINK_L4`
- `LINK_L5`
- `WHEEL_RADIUS`
- `WHEEL_GEAR_RATIO`
- `JOINT0_OFFSET`
- `JOINT1_OFFSET`
- `JOINT2_OFFSET`
- `JOINT3_OFFSET`
- `THETA_OFFSET_R`
- `THETA_OFFSET_L`
- `LEG_LENGTH_MIN`
- `LEG_LENGTH_MAX`
- `LEG_LENGTH_DEFAULT`

职责定义：

- 描述机器人几何与机构常量
- 被 controller、observer、kinematics、SITL adapter 共享
- 不表达控制策略选择

### B. control tuning params

建议放入：
[runtime/control/config/balance_control_params.h](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/config/balance_control_params.h)

建议迁移的参数：

- `LEG_GRAVITY_COMP`
- `LEG_PID_*`
- `TP_PID_*`
- `TURN_PID_*`
- `ROLL_PID_*`
- `PITCH_FALL_THRESHOLD`
- `PITCH_RECOVER_THRESHOLD`
- `FN_GROUND_THRESHOLD`
- `FN_WHEEL_GRAVITY`
- `LQR_K_MATRIX`

职责定义：

- 描述平衡控制器和相关状态机的调参与判据
- 允许未来按机器人 profile 或模式拆分
- 不暴露为 app 配置层入口

### C. operator / remote mapping params

建议放入：
[runtime/app/balance_chassis/app_config/remote_mapping_params.h](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/app/balance_chassis/app_config/remote_mapping_params.h)

建议迁移的参数：

- `RC_VX_MAX`
- `RC_TO_VX`
- `RC_TO_TURN`
- `RC_SPEED_SLOPE`

后续可继续放入：

- 摇杆 deadzone
- switch 到 mode 的映射
- recover / enable / safe-state 的操作策略

职责定义：

- 描述“操作者输入如何映射到机器人目标量”
- 属于 app/operator interface 语义
- 不应和 controller tuning 混在一起

### D. actuator / safety limits

建议放入：
[runtime/control/config/actuator_limits.h](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/config/actuator_limits.h)

建议迁移的参数：

- `JOINT_TORQUE_MAX`
- `WHEEL_TORQUE_MAX`
- `M3508_TORQUE_TO_CURRENT`
- `M3508_RPM_TO_RADS`
- `WHEEL_TORQUE_RATIO`
- `TURN_TORQUE_RATIO`

职责定义：

- 描述执行器能力边界和控制输出换算边界
- 被 `actuator_constraints` 和 `actuator_gateway` 共享
- 不直接绑定某个 controller 实现细节

## 当前代码迁移映射

### 第一批直接受影响文件

[runtime/app/balance_chassis/app_intent/remote_intent.c](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/app/balance_chassis/app_intent/remote_intent.c)

- 当前依赖：`balance_params.h`
- 迁移后依赖：
  - `robot_physical_params.h`
  - `remote_mapping_params.h`

[runtime/control/controllers/balance_controller.c](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/controllers/balance_controller.c)

- 当前依赖：`balance_params.h`
- 迁移后依赖：
  - `robot_physical_params.h`
  - `balance_control_params.h`
  - `actuator_limits.h`

[runtime/control/constraints/actuator_constraints.c](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/constraints/actuator_constraints.c)

- 当前依赖：`balance_params.h`
- 迁移后依赖：
  - `actuator_limits.h`

[runtime/module/algorithm/VMC/VMC_calc.h](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/module/algorithm/VMC/VMC_calc.h)

- 当前依赖：`balance_params.h`
- 迁移后依赖：
  - `robot_physical_params.h`

### 第二批可选优化文件

[runtime/control/state/chassis_observer.c](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/state/chassis_observer.c)

- 如果后续引入与几何/轮径直接相关的推导，应只依赖 `robot_physical_params.h`

[runtime/control/execution/actuator_gateway.c](/home/xbd/worspcae/code/Astra_RM2025_Balance/robot_platform/runtime/control/execution/actuator_gateway.c)

- 若未来加入映射/极限约束，可只依赖 `actuator_limits.h`

## 迁移步骤

### Phase A: 语义拆分，不改行为

1. 新建四个参数头文件
2. 将现有宏原样迁入新文件
3. 暂时保留 `balance_params.h`，但改为 deprecated 聚合头
4. 不改任何参数值

成功标准：

- 编译通过
- SITL smoke 通过
- Phase 1 / Phase 2 验证继续通过

### Phase B: include 边界迁移

1. 修改 `remote_intent.c` 的 include
2. 修改 `balance_controller.c` 的 include
3. 修改 `actuator_constraints.c` 的 include
4. 修改 `VMC_calc.h` 的 include
5. 删除不再需要的 `robot_def.h` 引用链

成功标准：

- 上述文件不再依赖 `control/internal/balance_params.h`
- `robot_def.h` 不再承担参数出口角色

### Phase C: 删除遗留聚合头

1. 删除 `robot_def.h`
2. 删除 `balance_params.h`，或仅保留短期兼容 shim 并明确标记弃用
3. 检查是否仍存在 app 层直接 include `control/internal/*`

成功标准：

- 项目内不存在新的 `control/internal/balance_params.h` 引用
- 参数边界符合目录语义

## 推荐的代码规则

建议后续明确约束：

- `app_config/` 只能放 robot-specific config 与 operator mapping
- `control/config/` 只能放控制器与安全阈值配置
- `control/internal/` 不再允许放项目级参数常量头
- app 层不得通过“转发头”间接 include controller internal 配置

## 为什么这件事对 SITL 也重要

这次拆分不只是“整理头文件”，它会直接影响 SITL 的可信度。

如果参数边界不清晰，SITL 很容易继续变成：

- 一套控制器内部常量和 app 配置混杂的运行方式
- 一套很难按机器人 profile、操控策略、控制策略独立替换的工程

拆完之后，SITL 的口径会更清楚：

- `robot physical definition`：描述机器人本体
- `control tuning params`：描述控制策略
- `remote mapping params`：描述人机输入映射
- `actuator limits`：描述执行器边界

这样主机上的 runtime 才是在运行“同一台机器人 + 同一套控制策略”，而不是运行“一份难以说明语义的宏集合”。

## 建议的下一步

建议先做最小重构：

1. 新建四个参数头
2. 迁移 `remote_intent.c`
3. 迁移 `balance_controller.c`
4. 迁移 `actuator_constraints.c`
5. 跑：
   - `python3 -m robot_platform.tools.platform_cli.main verify phase1 --project balance_chassis`
   - `python3 -m robot_platform.tools.platform_cli.main verify phase2 --project balance_chassis`

这一步完成后，再决定是否继续把参数从宏升级成结构体 profile。
