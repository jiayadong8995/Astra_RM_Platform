# platform_cli

建议的统一命令入口：

```text
platform generate
platform build
platform flash
platform debug
platform replay
platform sim
platform test
```

当前阶段只定义协议，不提供完整实现。

## 命令职责

- `generate`: 根据 yaml 调生成器后端，更新 `runtime/generated`
- `build`: 选择 `hw / replay / sim` 目标构建
- `flash`: 烧录硬件目标
- `debug`: 启动调试
- `replay`: 运行日志回放
- `sim`: 运行仿真场景
- `test`: 统一回归入口

