# Actuator Runtime Service

该目录承接执行器运行时链路：

- actuator command 输入
- 反馈采集
- 命令下发
- 启停与运行时保护

当前第一批迁入文件：

- `actuator_runtime.*`
- `actuator_topics.*`
- `motor_control_task.*`

后续继续把设备对象和句柄依赖往 `runtime/device` 收。
