# Phase 1 Bootstrap

## 目标

这一阶段只做平台入口，不动现有 `Chassis/` 和 `Gimbal/` 主线。

注意：

`Phase 1` 的前置条件是 `Phase 0` 先把纯 `CubeMX` 生成链跑通。

## 当前已经固定的约束

### 1. 平台入口

新平台入口固定为：

- `robot_platform/CMakeLists.txt`
- `robot_platform/projects/*`
- `robot_platform/tools/platform_cli/*`

### 2. 目录边界

运行时目录固定为：

```text
runtime/
  generated/
  bsp/
  module/
  app/
  osal/
```

### 3. 旧工程定位

旧工程：

- `Chassis/MDK-ARM/*.uvprojx`
- `Gimbal/MDK-ARM/*.uvprojx`

只作为迁移参考，不再作为主构建入口。

### 4. 第二阶段入口

第二阶段优先接 `Chassis`：

- generated 来源：`Chassis/Core`, `Chassis/Drivers`, `Chassis/Middlewares`
- bsp 来源：`Chassis/User/Bsp`
- module 来源：`Chassis/User/Algorithm`, `Chassis/User/Controller`, `Chassis/User/modules`
- app 来源：`Chassis/User/APP`

## 本阶段完成标准

1. 新平台目录稳定
2. toolchain 和项目配置有落点
3. `Chassis` 导入清单写清楚
4. 后续不需要再先写方案文档才能开工
