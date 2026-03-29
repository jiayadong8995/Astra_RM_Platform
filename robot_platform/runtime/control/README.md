# runtime/control

这一层承接运行时主控制链：

- `state`
- `controllers`
- `constraints`
- `execution`
- `contracts`

它不负责：

- 板级访问
- 设备驱动
- 业务模式管理

当前这一层已经开始承接真实实现：

- `state`
  - `ins_state_estimator`
  - `chassis_observer`
- `controllers`
  - `balance_controller`
- `execution`
  - `actuator_gateway`
  - `motor_control_task`

原则上：

- `state` 负责 `DeviceInput -> RobotState`
- `controllers` 负责 `RobotState + RobotIntent -> ActuatorCommand`
- `execution` 负责 `ActuatorCommand -> DeviceCommand/执行反馈`
