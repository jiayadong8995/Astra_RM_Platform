# runtime/control/constraints

这一层负责：

- 限幅
- 保护
- 安全裁剪

它对输入输出的要求是：

- 输入和输出都仍保持 `ActuatorCommand` 语义
- 不把约束层做成设备发送层
