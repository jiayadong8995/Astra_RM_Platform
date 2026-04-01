# runtime

运行时代码当前按以下目标层次推进。`robot_platform/docs/balance_chassis_bringup.md` is the authoritative bring-up reference for the blessed `balance_chassis` startup path.

```text
runtime/
  generated/
  bsp/
  device/
  control/
  app/
```

各层职责：

- `generated`
  - CubeMX/HAL 生成资产
- `bsp`
  - 板级支持与后端底层访问
- `device`
  - 统一设备语义
  - 当前目标子层：`imu / remote / actuator`
- `control`
  - 状态形成、控制求解、约束和执行映射
- `app`
  - 生命周期、模式管理、任务组织和业务装配
  - authoritative `balance_chassis` startup wiring 与项目装配

当前原则：

- 不再引入 `osal/` 作为目标层
- 不再围绕所谓 legacy 目录组织整体架构
- 主控制链优先围绕正式 contracts 迁移
- `hw` 与 `sitl` 的具体设备实现必须分离
- `app` owns project composition and startup wiring, while `control` owns the observe -> control -> execution chain
- `balance_chassis` is the proving path for the reusable platform, not a bypass around `runtime/device` or `runtime/control`
