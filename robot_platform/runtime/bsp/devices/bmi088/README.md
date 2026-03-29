# bmi088 device

这里保留 BMI088 底层驱动和中间件，并补了：

- `bmi088_node.[ch]`

`bmi088_node` 的职责是：

- 收口默认 SPI 句柄
- 收口采样状态对象
- 收口初始化/读取函数

供 `runtime/device/imu` 绑定默认 backend 配置。
