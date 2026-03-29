# module

这里承接跨机器人可复用的通用能力：

- 算法
- 控制小原件
- 公共工具
- 消息总线

当前典型内容包括：

- `algorithm/`
  - EKF / PID / VMC / mahony / kalman
- `lib/control/`
  - `control_math`
- `message_center/`
  - 轻量消息缓存

这一层不承接机器人专属控制组合逻辑。
