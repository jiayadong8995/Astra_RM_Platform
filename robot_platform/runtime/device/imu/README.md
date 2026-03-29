# runtime/device/imu

这一层定义 IMU 设备语义。

它向上提供：

- 原始 `imu_sample`
- 设备健康状态

它不负责：

- 姿态解算
- 观测融合
- 业务判断
