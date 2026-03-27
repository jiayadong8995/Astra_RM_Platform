# sim

当前第一版已经支持最小 physics sim 场景运行。

目录：

- replay
- physics
- adapters
- scenarios
- reports

当前命令：

```bash
python3 -m robot_platform.tools.platform_cli.main sim
python3 -m robot_platform.tools.platform_cli.main sim standstill
python3 -m robot_platform.tools.platform_cli.main sim push_recovery
python3 -m robot_platform.tools.platform_cli.main sim tilt_recovery
```

当前这版 sim 的定位：

- 先验证平台命令、场景格式、最小物理闭环和报告输出
- 先不直接执行 legacy `Chassis` 控制链
- 后续再把 replay 和 control adapter 接进来
