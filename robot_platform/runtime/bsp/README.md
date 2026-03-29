# bsp

这里承接板级访问和设备节点。

当前 `bsp` 只做两类事：

- `boards/`
  - 板级总线访问
  - 时基 / PWM / UART / CAN 等底层能力
- `devices/`
  - 面向具体驱动的 backend 节点
  - 给 `runtime/device` 提供默认绑定配置

它不负责：

- 业务状态
- 控制求解
- app 编排
