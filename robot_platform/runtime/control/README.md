# runtime/control

这一层承接运行时主控制链：

- `state`
- `controllers`
- `constraints`
- `execution`
- `contracts`

它不负责：

- 板级访问
- 设备驱动
- 业务模式管理

当前第一轮只先建立正式契约头文件，作为后续迁移的代码落点。
