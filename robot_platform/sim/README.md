# sim

当前 `sim/` 目录只保留 SITL 主线所需资产。

目录：

- replay
- adapters
- bridge
- reports

当前命令：

```bash
python3 -m robot_platform.tools.platform_cli.main build sitl
./build/robot_platform_sitl_make/balance_chassis_sitl
python3 -m robot_platform.sim.bridge.sim_bridge
```

当前这版 sim 的定位：

- 先把 legacy 控制链跑进 Linux 进程
- 先把 BSP socket bridge 跑通
- `physics_sim` 暂不作为当前资源投入方向
