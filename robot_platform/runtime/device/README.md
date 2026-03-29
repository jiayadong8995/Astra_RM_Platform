# runtime/device

`runtime/device` 是统一设备语义层。

它的职责只有三个：

- 承接具体设备驱动与设备语义
- 把底层驱动包装成稳定设备接口
- 向上提供设备输入、反馈、命令能力
- 屏蔽板级后端差异

backend profile 装配现在只作为 `device_layer` 的内部依赖存在，源码位于：

- `device_profile_hw.c`
- `device_profile_sitl.c`

它们内部按设备域拆成：

- `bind_*_imu`
- `bind_*_remote`
- `bind_*_motor`

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

当前对上正式暴露的只应是：

- `device_layer`
- 设备子域的稳定设备语义接口
- `device_profile`

这里的 `profile` 是当前 `device` 层的策略模型：

- 一个 profile 定义一套设备节点装配方式
- `hw` 和 `sitl` 通过不同 profile 装配同一套 `device_layer`
- `device_layer` 不再直接依赖“默认绑定函数名”

这里的“设备节点”指的是：

- `platform_imu_device_t`
- `platform_remote_device_t`
- `platform_motor_device_t`

`device_layer` 负责持有节点，`device_profile` 负责决定这些节点绑定哪套 concrete driver。

当前默认 profile 规则：

- `hw` 构建默认选择 `PLATFORM_DEVICE_BACKEND_PROFILE_HW`
- `sitl` 构建默认选择 `PLATFORM_DEVICE_BACKEND_PROFILE_SITL`
- 如需切换，应通过 `platform_device_configure_default_profile(...)` 完成
