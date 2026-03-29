# runtime/device/actuator

这一层定义执行器设备语义。

`actuator` 是长期接口名，不限定具体执行器类型。

当前第一版只实现：

- `motor`

后续若引入其他执行器，应继续挂在 `actuator/` 下，而不是改动上层主架构。
