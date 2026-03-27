# Runtime Skeleton

```text
runtime/
  generated/
  bsp/
  module/
  app/
  osal/
```

规则：

- `generated/` 只放生成代码
- `bsp/` 只做设备和板级适配
- `module/` 不允许依赖 HAL
- `app/` 负责装配和调度
- `osal/` 提供运行环境抽象

