# runtime/control/execution

这一层负责：

- `ActuatorCommand -> DeviceCommand`

它是控制输出和设备命令之间的边界层。

它不负责：

- 业务模式管理
- 设备驱动实现
- HAL/BSP 直接访问
