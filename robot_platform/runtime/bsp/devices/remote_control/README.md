# remote_control device

这里保留 UART5 SBUS 遥控输入路径，并补了：

- `remote_node.[ch]`

Imported files:

- `remote_control.[ch]`
- `struct_typedef.h`
- `bsp_rc.h`

其中 `remote_node` 负责把：

- 遥控状态获取函数
- 遥控错误检测函数

收成默认 backend 节点，供 `runtime/device/remote` 使用。
