# runtime/control/state

这一层负责：

- `DeviceInput -> RobotState`
- 姿态形成
- 观测量整理
- 状态融合

它不负责：

- 模式管理
- 业务决策
- 设备驱动访问
