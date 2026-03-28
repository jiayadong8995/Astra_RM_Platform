# 01 Architecture

## 1. 设计目标

当前仓库的问题不是“功能不够”，而是“边界不硬”：

- `CubeMX` 生成代码和业务代码混合
- `Keil` 工程文件承担了真实构建入口职责
- 模块间大量使用全局变量和直接依赖
- 无法脱离板子验证控制链行为

平台化改造的目标是把工程拆成 4 条明确链路：

1. 生成链：配置 -> 生成代码
2. 构建链：源代码 -> 固件/仿真目标
3. 运行链：generated -> bsp -> module -> app
4. 验证链：hw / replay / physics_sim

## 2. 第一版平台范围

只覆盖当前 `Chassis` 的最小闭环：

- STM32H7 控制板
- `CubeMX` 生成层
- GCC/CMake 构建
- 关键 BSP 设备抽象
- balance chassis 主控制链
- 一条真实日志 replay
- 一个最小仿真闭环

`Gimbal` 暂不进入第一版迁移主线。

## 3. 新平台分层

```text
robot_platform/
  tools/
  runtime/
    generated/
    bsp/
    module/
    app/
    osal/
  sim/
    replay/
    physics/
    adapters/
  projects/
```

### 3.1 `runtime/generated`

职责：

- 存放 `CubeMX` 或 `CubeMX2` 生成代码
- 启动文件、链接脚本、中断桩、HAL 配置都归这里

约束：

- 禁止手写业务逻辑
- 可以被再生成覆盖
- 不作为长期人工维护层

### 3.2 `runtime/bsp`

职责：

- 设备和板级适配
- 把 HAL、外设句柄、总线细节封进统一接口

允许依赖：

- `generated`
- `HAL`

禁止依赖：

- 控制策略
- 业务状态机

### 3.3 `runtime/module`

职责：

- 算法
- 估计器
- 控制器
- 安全模块
- message bus / topic / daemon

允许依赖：

- `module`
- `osal`

禁止依赖：

- HAL
- 中断
- RTOS 任务句柄

### 3.4 `runtime/app`

职责：

- 模块装配
- 模式管理
- 生命周期
- 任务编排

允许依赖：

- `bsp`
- `module`
- `osal`

禁止依赖：

- 直接碰寄存器
- 直接操作 HAL handle

### 3.5 `runtime/osal`

职责：

- 时间
- 锁
- 队列
- task/thread 抽象

目的：

- 把 `module` 和平台运行环境隔离
- 同时服务 `hw / replay / sim`

## 4. 生成与构建策略

## 4.1 生成后端抽象

不要把平台绑定死到某一个生成器实现。第一版建议抽象为：

```text
yaml config -> internal board model -> generator backend -> generated tree
```

后端实现预留两种：

1. `stm32cubemx_ioc_backend`
2. `cubemx2_backend`

这样可以先用稳定方案落地，再逐步替换后端。

## 4.2 构建入口统一

新平台只认 `CMake`。

保留 `Keil uvprojx` 的唯一用途：

- 查旧工程 include path
- 查旧工程编译宏
- 查旧工程启动文件和链接设置

不再允许：

- 以 `Keil` 工程文件作为主构建入口
- 在 `uvprojx` 中独占维护源文件清单

## 5. 运行环境

## 5.1 `hw`

真实板卡运行。

特点：

- 真实传感器
- 真实电机总线
- 真实 RTOS

## 5.2 `replay`

读取实车日志，按时间推进，给 `app/module` 喂传感器数据。

特点：

- 能快速验证重构前后行为漂移
- 比 physics sim 更适合作为第一版验证前移工具

## 5.3 `physics_sim`

由简化动力学模型提供观测，接收控制输出。

特点：

- 适合做场景化回归
- 第一版只做最小闭环，不追求高保真

## 6. 对当前仓库的关键决策

### 决策 1

第一版只迁 `Chassis`，不同时迁 `Gimbal`。

### 决策 2

先做目录和边界迁移，再做算法重写。

### 决策 3

先做 `replay`，再做复杂 `physics_sim`。

### 决策 4

历史上的 `Framework/` 已从仓库中移除；相关思路只作为迁移背景，不再作为可复用资产保留。

### 决策 5

现有 `message_center`、`ARF topic`、全局变量数据流三套机制，后续必须收敛成一套。
