# runtime/control

这一层承接运行时主控制链。See `robot_platform/docs/balance_chassis_bringup.md` for the authoritative `app` versus `control` ownership split.

- `state`
- `controllers`
- `constraints`
- `execution`
- `contracts`

它不负责：

- 板级访问
- 设备驱动
- `app` 负责的业务模式管理
- 通用控制原件库
- project startup wiring

当前这一层已经开始承接真实实现：

- `state`
  - `ins_state_estimator`
  - `chassis_observer`
- `controllers`
  - `balance_controller`
- `execution`
  - `actuator_gateway`
  - `motor_control_task`

通用算法和控制小原件不再留在这一层，统一回到 `runtime/module`。

原则上：

- `state` 负责 `DeviceInput -> RobotState`
- `controllers` 负责 `RobotState + RobotIntent -> ActuatorCommand`
- `execution` 负责 `ActuatorCommand -> DeviceCommand/执行反馈`
- `app` owns remote intent ingress and project composition; `control` owns the runtime observe -> control -> execution path
