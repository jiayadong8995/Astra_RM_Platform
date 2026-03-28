# Runtime Device Layer

`runtime/device` 承接统一设备节点抽象。

当前阶段先立空层，后续按文档逐步补齐：

- `sensor/imu_device`
- `input/remote_control_device`
- `actuator/joint_actuator_device`
- `actuator/wheel_actuator_device`

原则：

- 向上暴露设备语义
- 向下封装具体 BSP/driver 差异
- 不承接业务装配和控制流程
