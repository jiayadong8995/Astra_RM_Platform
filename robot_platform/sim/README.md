# sim

当前 `sim/` 目录只保留 SITL 主线所需资产。

目录：

- replay
- adapters
- bridge
- reports

当前命令：

```bash
python3 -m robot_platform.tools.platform_cli.main sim
python3 -m robot_platform.tools.platform_cli.main sim --duration 5 --skip-build
python3 -m robot_platform.tools.platform_cli.main test sim
```

当前这版 sim 的定位：

- 先把 legacy 控制链跑进 Linux 进程
- 先把 BSP socket bridge 跑通
- `physics_sim` 暂不进入当前执行面

当前 `sim` 命令会：

- 自动构建 `balance_chassis_sitl`
- 拉起 `sim_bridge` 和 SITL 进程
- 执行一个最小 smoke session
- 输出 `build/sim_reports/sitl_smoke.json`

当前 smoke report 至少记录：

- 运行状态和进程退出信息
- runner 声明的 runtime 正式边界
- runner 声明的传输端口
- bridge 成功运行时观察到的 runtime 边界 / 传输端口
- bridge 基础统计样本（IMU 包、命令包、反馈包）
- 最小健康摘要（调度器启动、IMU 注入、命令/反馈活动）
- 顶层 smoke 结果摘要（是否通过、首个失败项、关键计数）
- 统计摘要（样本数、计数增量、单位时间速率）
- 可选警告项（例如已启动但尚未看到电机命令/反馈）
- bridge 失败时的 SITL 存活状态（区分“bridge 没起来”和“SITL 自己崩了”）

当前 `test sim` 会跑 sim 侧最小单元回归，锁住：

- bridge 事件解析
- 边界/端口声明比对
- stats 去重和统计摘要
- 失败路径 smoke 结果归因
