# Runtime Communication Matrix

这份文档定义运行时各层之间应采用的通信方式。

目的：

- 避免把“数据流”误当成“通信机制”
- 避免主控制链退化成全量 topic 化
- 固定哪些边界应该 direct call，哪些边界才允许广播

---

## 1. 核心原则

当前正式原则是：

1. 主控制闭环优先使用直接接口
2. 广播链才使用轻量 pub-sub
3. 请求式操作使用显式调用或 request-response
4. 不把消息总线作为主控制链唯一通信方式

---

## 2. 通信矩阵

| 上游 | 下游 | 传递对象 | 建议通信方式 | 原因 |
|---|---|---|---|---|
| `device` | `control/state` | `DeviceInput` / `DeviceFeedback` | 直接接口 | 强周期、强所有权、强实时 |
| `control/state` | `app` | `RobotState` | 直接接口为主 | app 每周期直接消费正式主状态 |
| `app` | `control/controllers` | `RobotIntent` | 直接接口 | 业务目标输入应明确且同步 |
| `control/controllers` | `control/constraints` | `ActuatorCommand` | 直接接口 | 控制链内部同步处理 |
| `control/constraints` | `control/execution` | `ActuatorCommand` | 直接接口 | 执行前裁剪仍属主闭环 |
| `control/execution` | `device` | `DeviceCommand` | 直接接口 | 后端执行需要明确命令所有权 |
| `RobotState` | `sim/logger/ui` | 状态广播 | 轻量 pub-sub | 多消费者、观测型场景 |
| fault/health | `app/ui/sim` | 健康状态 | 轻量 pub-sub | 广播型场景 |
| `app` | 生命周期/模式切换入口 | 模式切换请求 | 显式调用 / request-response | 不是持续流数据 |

---

## 3. 主控制链

主控制链建议表达为：

```text
device.read()
  -> control.state.update()
  -> app.intent.update()
  -> control.controllers.update()
  -> control.constraints.update()
  -> control.execution.map()
  -> device.write()
```

说明：

- 这是主控制闭环
- 这条链上的默认机制是直接接口
- 这条链不应被拆成大量异步 topic 传递

---

## 4. 允许广播的场景

以下场景适合使用轻量 pub-sub：

1. 状态广播
   - `RobotState` 给 `sim`
   - `RobotState` 给日志
   - `RobotState` 给 UI

2. 健康广播
   - 故障状态
   - 退化模式
   - 在线状态

3. 调试观测
   - 某些中间诊断量
   - 非主控制必需的观测量

广播使用约束：

- 广播对象必须是正式对象或明确标记为诊断对象
- 广播不应替代主控制链上的正式接口

---

## 5. 不建议使用 pub-sub 的场景

以下场景默认不建议使用 pub-sub：

1. `device -> control/state`
   - 设备输入属于单一所有权主链输入

2. `app -> control`
   - `RobotIntent` 是正式同步业务目标

3. `control -> execution`
   - 控制输出和执行映射之间属于强依赖同步过程

4. `execution -> device`
   - 设备命令必须保持明确所有权和时序

---

## 6. 请求式交互

以下场景建议使用显式调用或 request-response：

1. 模式切换
2. 生命周期切换
3. 校准
4. 重新初始化
5. 人工触发类操作

原因：

- 这些不是持续主链数据
- 它们更接近命令和事务，而不是状态流

---

## 7. 当前阶段结论

当前项目后续实现必须遵守：

- 主控制链：直接接口
- 广播链：轻量 pub-sub
- 请求式操作：显式调用

这三者必须分开理解。

后续任何实现如果试图把主控制链重新做成“全量 topic 化”，都应视为偏离目标架构。
