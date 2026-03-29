# dm_motor device

这里保留 DM4310 / 轮电机底层协议实现，并补了：

- `motor_node.[ch]`

`motor_node` 的职责是：

- 把底层句柄、getter、命令函数收成 backend 节点
- 供 `runtime/device/actuator/motor` 绑定默认配置
