# runtime/device/actuator/motor

这一层定义 motor 类执行器的统一语义。

当前范围覆盖：

- 关节电机
- 轮电机

两者目前先放在同一语义域下，由 `platform_motor_kind_t` 区分。
