# runtime/control/primitives

这一层放 `control` 内部复用原件。

当前已放入：

- `control_math.*`
  - 浮点限幅
  - `int16` 限幅
  - 目标斜坡跟随

后续继续放：

- 通用滤波器
- 控制数学工具
- 不带机器人业务语义的控制小原件

这一层放控制内部复用原件，例如：

- PID
- EKF
- Mahony
- VMC 基础实现
- 数学工具

原则：

- 主要服务 `control` 层内部
- 不作为 `app` 直接依赖的业务接口
