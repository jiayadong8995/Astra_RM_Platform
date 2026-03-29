# runtime/device

`runtime/device` 是统一设备语义层。

它的职责只有三个：

- 承接具体设备驱动与设备语义
- 把底层驱动包装成稳定设备接口
- 向上提供设备输入、反馈、命令能力
- 屏蔽板级后端差异

默认后端绑定现在统一收口在：

- `device_backend_hw.c`
- `device_backend_sitl.c`

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
- app 编排

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

当前第一版只把 `motor` 作为 `actuator` 的具体实现落下来。
原先 `runtime/bsp/devices` 中的驱动资产已经并入这一层，避免“驱动目录”和“设备语义目录”双重并存。

下一步还需要继续收紧：

- `device` 对上只暴露稳定设备语义
- `app / control` 不应继续直接依赖具体驱动头
- backend 默认绑定应继续压缩到最少暴露面
