# runtime/device

`runtime/device` 是统一设备语义层。

它的职责只有三个：

- 把底层驱动包装成稳定设备接口
- 向上提供设备输入、反馈、命令能力
- 屏蔽 BSP/driver 差异

它的实现原则还有一条：

- 设备接口可以统一，但 concrete adapter 必须按 backend 分离

也就是说：

- `hw` 的设备实现可以依赖真实驱动和板级句柄
- `sitl` 的设备实现必须单独提供
- 不再允许一份 concrete device 同时硬绑 `hw + sitl`

它不负责：

- 状态形成
- 控制求解
- 业务模式管理

当前目标结构：

```text
runtime/device/
  device_types.h
  imu/
  remote/
  actuator/
    actuator_device.h
    motor/
```

当前第一版只把 motor 作为 actuator 的具体实现落下来。
