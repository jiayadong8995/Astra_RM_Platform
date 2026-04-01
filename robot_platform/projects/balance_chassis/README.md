# balance_chassis

这是当前平台唯一保留的项目配置目录，也是当前可复用平台设计的 proving path。

它描述的是第一版平台化迁移主对象：

- legacy 底盘控制链
- 对应控制板
- 当前主运行模式

当前目录只保留项目配置，不放真实业务源码。

这意味着它现在的职责是“项目描述入口”，不是“项目实现目录”。

权威 bring-up 文档见 `robot_platform/docs/balance_chassis_bringup.md`。

`balance_chassis` 是 reusable platform 的 proving path，不是绕过 `runtime/device` 或 `runtime/control` 的 one-off bypass。
