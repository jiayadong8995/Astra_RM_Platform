# StandardRobotpp Reference Notes

来源仓库：<https://gitee.com/SMBU-POLARBEAR/StandardRobotpp>

同步信息：

- upstream commit: `df57ee727d155cc0de7e975f11f3d9a13d1da79e`
- upstream date: `2025-11-10 14:41:39 +0000`

该参考仓库适合关注的点：

- `application / components / bsp` 三层目录划分比较清晰，适合对照我们当前的平台分层。
- `application/*_task.c` 和对应模块实现的分离比较明确，适合作为任务入口与控制逻辑解耦参考。
- `application/robot_param*.h` + `application/typedef/robot_typedef.h` 使用“车型参数文件 + 预编译硬件类型选择”的方式做机器人适配，适合参考配置收口方式。
- `application/chassis` 对平衡、麦轮、全向轮、舵轮提供了并列实现，适合参考同一模块下多硬件形态的组织方式。
- `application/robot_cmd` 收口多种执行器与 CAN/PWM 下发路径，适合参考执行机构抽象边界。
- `components/controller`、`components/algorithm`、`components/devices` 的拆分比较直接，适合参考控制器、算法和传感器驱动的归属边界。

结合当前仓库，建议优先参考这些部分：

- 若关注嵌入式平台分层：看 `bsp/boards`、`components/*`、`application/*`
- 若关注任务模型：看 `application/chassis/chassis_task.c`、`application/gimbal/gimbal_task.c`、`application/shoot/shoot_task.c`
- 若关注产品变体适配：看 `application/robot_param.h` 以及各类 `application/robot_param_*.h`
- 若关注平衡底盘实现：看 `application/chassis/chassis_balance*.c`

不建议直接照搬的部分：

- 工程当前以 `Keil5 + STM32CubeMX + STM32F4 HAL` 为中心，和我们主线的 GCC/CMake/统一生成链并不一致。
- 目录内包含较强的板级和比赛项目耦合，适合作为组织方式参考，不适合作为直接集成依赖。
