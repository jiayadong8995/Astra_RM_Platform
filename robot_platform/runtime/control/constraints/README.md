# runtime/control/constraints

这一层放控制输出和行为边界约束。

当前已放入：

- `actuator_constraints.*`
  - 关节/轮输出限幅
  - 遥控转向窗口约束
  - 腿长调整步幅约束

原则：

- 约束属于 `control`
- 不属于 `app`
- 不属于 `device`

这一层负责：

- 限幅
- 保护
- 安全裁剪

它对输入输出的要求是：

- 输入和输出都仍保持 `ActuatorCommand` 语义
- 不把约束层做成设备发送层
