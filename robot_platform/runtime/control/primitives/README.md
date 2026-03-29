# runtime/control/primitives

这一层放控制内部复用原件，例如：

- PID
- EKF
- Mahony
- VMC 基础实现
- 数学工具

原则：

- 主要服务 `control` 层内部
- 不作为 `app` 直接依赖的业务接口
