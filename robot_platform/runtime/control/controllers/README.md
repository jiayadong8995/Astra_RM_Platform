# runtime/control/controllers

这一层负责：

- `RobotState + RobotIntent -> ActuatorCommand`
- 机器人专属控制组合逻辑

它不负责：

- 设备驱动
- 板级发送
- 生命周期管理
