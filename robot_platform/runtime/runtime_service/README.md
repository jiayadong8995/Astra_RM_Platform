# Runtime Service Layer

`runtime/runtime_service` 承接设备层与控制链之间的运行时 glue。

当前阶段按文档先建立四条服务线：

- `sensing/`
- `observe/`
- `actuator/`
- `control_runtime/`

原则：

- 负责输入整理、反馈采集、运行时状态映射
- 不承担业务装配
- 不直接暴露板级实现细节给 `app`
