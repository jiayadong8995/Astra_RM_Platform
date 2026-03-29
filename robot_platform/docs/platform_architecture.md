# Platform Architecture

这份文档定义当前项目的统一口径：

- 这个项目是什么
- 新平台当前做到哪一步
- 新平台的目标架构是什么
- 当前演进目标和边界是什么
- 当前阶段应该做什么，不做什么

它不是历史讨论稿，而是当前继续推进项目时应参考的架构说明。

## 1. 项目定义

`Astra_RM_Platform` 是一个嵌入式平台架构仓库。

它的目标不是继续维护一份以 `CubeMX + Keil + legacy task` 为中心的单体固件，而是基于已有历史基线，定义并实现统一的平台体系：

- 统一生成链
- 统一构建链
- 统一运行时分层
- 统一验证链

整个仓库由三类资产组成：

1. `Astra_RM2025_Balance/`
   历史基线资产。
   保存原始固件、`.ioc`、驱动布局和行为参考，不再作为主开发入口。
2. `robot_platform/`
   当前主工程。
   负责承接生成、构建、运行时分层和当前验证主线，是平台继续演进的执行面。
3. `Platform_Refactor/`
   路线决策归档。
   只解释为什么最后选了现在这条路线，不承担日常开发文档职责。

## 2. 当前状态

### 2.1 已经成立的部分

当前仓库已经具备以下基础能力：

1. `STM32CubeMX` 生成链已经接入 CLI
2. `CMake + arm-none-eabi-gcc` 的硬件构建链已经可用
3. `balance_chassis_hw_seed.elf` 可构建
4. `linux-gcc + FreeRTOS POSIX port` 的 SITL 目标 `balance_chassis_sitl` 可构建
5. `runtime/bsp/sitl` 和 `sim/bridge` 已经具备最小 SITL 骨架

### 2.2 仍未闭环的部分

当前还没有完全收口的部分主要有：

1. `flash / debug / replay / test` 还没有统一 CLI 闭环
2. 新消息总线体系还没有成为当前主控制链的统一消息机制
3. `runtime/module` 仍存在对 legacy 头文件和板级实现的直接依赖
4. `runtime/app/balance_chassis` 仍主要承接 legacy task，而不是全新平台化编排

## 3. 新平台当前架构

当前平台在代码层面已经形成下面这套骨架：

```text
robot_platform/
  cmake/
  docs/
  projects/
  runtime/
    generated/
    bsp/
    module/
    app/
  sim/
  tools/
```

各层职责如下。

### 3.1 `tools/`

职责：

- 提供统一入口
- 驱动生成和构建
- 后续承接 flash / debug / replay / test

当前状态：

- `generate` 已接 `CubeMX`
- `build hw_elf / sitl` 已可用
- 其余命令仍在收口中

### 3.2 `runtime/generated/`

职责：

- 承接 `CubeMX` 生成代码
- 保存启动文件、外设初始化、HAL 配置等底层生成资产

当前状态：

- 当前使用 `stm32h7_ctrl_board_raw`
- 这是当前硬件构建链的重要输入

约束：

- 不应继续塞入长期人工维护的业务逻辑

### 3.3 `runtime/bsp/`

职责：

- 承接板级和设备驱动
- 把底层硬件访问收口在可管理边界内

当前状态：

- 已有板级目录 `boards/stm32h7_ctrl_board`
- 已有 BMI088、DM 电机、遥控器等设备目录
- 已有 `sitl/` 目录承接 Linux 侧 BSP 桩实现

### 3.4 `runtime/module/`

职责：

- 放算法、控制器、公共模块
- 承接可复用的控制逻辑

当前状态：

- 已有 `PID`、`VMC`、`mahony`、`kalman`、`EKF`、`controller`、`message_center`
- 但还没有完全从 legacy 头文件和板级依赖中脱开

这意味着它现在还是“已经进入平台结构的模块层”，还不是“完全平台无关的模块层”。

### 3.5 `runtime/app/`

职责：

- 负责业务装配、任务编排、模式管理
- 承接具体机器人项目入口

当前状态：

- `balance_chassis` 已经承接当前控制主链，不再保留独立 seed app 目录
- `balance_chassis` 已承接 legacy task 主体
- 当前已经形成平台 app 的第一版落位，但 app 边界和编排方式还在继续收紧

### 3.6 `projects/`

职责：

- 承接项目配置
- 描述 board、robot、runtime mode 和 app 入口

当前状态：

- 当前主项目为 `balance_chassis`
- `runtime_modes` 已收口为 `hw` 和 `sitl`

### 3.7 `sim/`

职责：

- 承接 SITL、bridge、replay、report

当前状态：

- 当前主线是 `SITL`
- `bridge` 和 `reports` 有骨架
- `replay` 保留为后续能力，不作为当前交付项
- `physics_sim` 当前不进入执行面

## 4. 目标架构预演

新平台的目标不是一次性推翻 legacy，而是在现有平台骨架上收紧边界，最终形成下面的结构：

```text
project config
  -> generate
  -> build
  -> runtime
       generated -> bsp -> module -> app
  -> validation
       hw / sitl
```

可以把它拆成四条主链路理解。

### 4.1 生成链

目标：

- 由 `CubeMX` 生成底层代码
- 输出进入 `runtime/generated`
- 不再把 IDE 工程文件当成真实入口

### 4.2 构建链

目标：

- 统一走 `CMake`
- 硬件侧走 `arm-none-eabi-gcc`
- SITL 侧走 `linux-gcc`
- 后续 flash / debug 也从这里接入

### 4.3 运行链

目标：

- `generated` 负责生成产物
- `bsp` 负责硬件访问
- `module` 负责控制逻辑
- `app` 负责装配和调度

这条链最终要做到：

- `hw` 与 `sitl` 使用同一条主控制链
- 差异主要留在 `bsp` 和环境注入层

### 4.4 验证链

目标：

- `hw` 用于真实板级运行
- `sitl` 用于在 Linux 进程中运行完整控制链

当前验证主线是：

`hw <-> sitl`

`replay` 仍有价值，但当前不进入交付面，后续再作为扩展验证能力接入。

## 5. 当前演进目标

当前目标不是“把所有 legacy 文件换个目录再放一遍”，而是逐步完成这五件事：

1. 用统一生成链接管底层生成入口
2. 用统一构建链接管硬件与 SITL 构建入口
3. 把主控制链收进平台运行时目录
4. 用 `SITL` 形成当前最小验证闭环
5. 推进新消息总线体系成为统一消息机制

当前第一阶段聚焦对象只有一个：

- `balance_chassis`

也就是先把底盘控制链在当前平台架构下跑稳，不同时展开 `gimbal` 或更多项目。

## 6. 当前边界

为了避免项目继续发散，当前边界必须明确。

### 6.1 当前要做的事

- 保持 `hw_elf` 和 `sitl` 双构建链稳定
- 推进新消息总线体系全面落地
- 继续收紧 `module` 和 `app` 的边界
- 把 `flash / debug / test` 收进统一 CLI

### 6.2 当前不做的事

- 不新建 `osal` 层
- `replay` 暂不作为当前交付项
- 不做 `physics_sim`
- 不做高保真场景库
- 不做大范围算法重写
- 不同时平台化多个业务项目

## 7. 当前关键技术判断

### 7.1 为什么 `robot_platform` 是主入口

因为它已经承接了：

- 生成
- 构建
- 运行时分层
- SITL 主验证路线

所以继续开发应围绕 `robot_platform/` 展开。

### 7.2 为什么 `Astra_RM2025_Balance` 只保留为基线

因为它的价值已经从“主工程”转成了：

- 原始行为参考
- 原始代码参考
- `.ioc` 和驱动资产来源

它不应再承担平台主开发职责。

### 7.3 为什么当前主验证路线是 `SITL`

因为当前资源条件下，这条路最现实：

- `SITL` 能运行完整控制链
- 它比 `replay` 和 `physics_sim` 更适合作为当前第一交付
- 它能先验证平台化之后控制链在 Linux 侧真实跑起来

`replay` 仍然有价值，但当前排期后置。

## 8. 当前阶段的架构目标

如果只看接下来一段时间，平台架构目标可以压缩成一句话：

在不引入新抽象层和新验证分支的前提下，围绕 `balance_chassis` 把平台运行时边界和新消息总线体系收紧下来，并先把 `hw + sitl` 这条最小平台闭环跑稳。

达到这个目标后，项目才算真正进入“可持续演进的平台工程”。

## 9. 当前正式对接面

为了让 `sim / bridge / report / replay` 不再依赖 legacy 内部全局变量，当前阶段先把 runtime 对外边界固定如下。

### 9.1 正式输入 topic

- `ins_data`
- `chassis_cmd`

说明：

- `ins_data` 是姿态与惯导输入边界
- `chassis_cmd` 是遥控/上层控制命令输入边界

### 9.2 正式输出 topic

- `robot_state`
- `actuator_command`

说明：

- `robot_state` 是当前对外状态摘要
- `actuator_command` 是当前控制主链的正式执行输出边界
- 在当前阶段，`sim` 应优先接这两类输出，而不是读取 `chassis_move`、`left/right` 一类内部全局状态

### 9.3 过渡 topic

- `chassis_observe`
- `device_feedback`

说明：

- `chassis_observe` 当前仍属于控制链内部中间量
- 它用于把 `observe_task` 的估计结果显式送入控制链
- `device_feedback` 是当前执行层回流到控制层的正式设备反馈 topic
- 现阶段不应把这些内部执行 topic 当成长期稳定的外部报告边界固化到 replay/report 协议中，除非后续明确升级为正式观测接口

### 9.4 禁止的对接方式

当前确认不应作为 `sim` 正式对接面的内容：

- `INS`
- `chassis_move`
- `left / right`
- `rc_ctrl`

原则：

- `hw` 与 `sitl` 共用同一条主控制链
- 环境差异只允许留在 `bsp` 和注入层
- `sim` 不应为了读状态或写输入而直接依赖 app 内部结构
