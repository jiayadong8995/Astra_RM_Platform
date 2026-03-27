# Astra_RM2025_Balance 底盘重构指南

## 已完成

- [x] **Step 0**: GBK→UTF-8 编码转换 (19 文件) + .editorconfig + .gitignore
- [x] **Step 1**: 创建 `robot_def.h`，收拢全部 magic number

---

## Step 2: 合并 VMC 左右腿 + 修复左腿滤波 bug

**目标**: 消除 `VMC_calc_1_right` / `VMC_calc_1_left` 的代码重复，同时修复左腿缺失滤波的 bug。

**现状问题**:
- `VMC_calc_1_left` 缺少 `d_phi0` 一阶滤波、`dd_L0` 一阶滤波、`dd_theta` 一阶滤波
- `VMC_calc_1_left` 的 `first_flag` 分支没有初始化 `last_d_phi0`、`last_d_L0`、`last_dd_L0`、`last_d_theta`、`last_dd_theta`
- 两个函数 95% 代码相同，只有 pitch 符号和 theta_offset 不同

**操作**:
1. 合并为统一函数:
   ```c
   void VMC_calc_1(vmc_leg_t *vmc, float pitch, float gyro_pitch, float dt, float theta_offset);
   ```
2. 调用处改为:
   ```c
   VMC_calc_1(&right, INS.Pitch,  INS.Gyro[0], dt, THETA_OFFSET_R);
   VMC_calc_1(&left, -INS.Pitch, -INS.Gyro[0], dt, THETA_OFFSET_L);
   ```
3. 同理合并 `ground_detectionR` / `ground_detectionL` 为 `ground_detection(vmc_leg_t *vmc)`
4. 删除 `User/APP/remote_control.c/h`（与 `User/Devices/Remote_Control/` 下完全重复）

**验证**: 合并后左腿自动获得完整滤波链，左右腿行为对称。

---

## Step 3: 引入 message_center 发布-订阅

**目标**: 用 pub-sub 替代 extern 全局变量，解耦模块间数据流。

**现状问题**:
- `chassis_move`、`right`、`left`、`INS`、`rc_ctrl` 通过 extern 被 5+ 个文件直接访问
- 任何文件都能读写任何全局状态，无法追踪数据流向

**操作**:
1. 从 basic_framework 移植 `modules/message_center/message_center.c/h`
2. 定义 topic 枚举和数据结构:
   ```c
   // robot_def.h 中添加
   typedef enum {
       TOPIC_INS_DATA,      // INS_t
       TOPIC_RC_DATA,       // RC_ctrl_t
       TOPIC_CHASSIS_CMD,   // chassis_cmd_t (v_set, turn_set, leg_set, jump_flag...)
       TOPIC_CHASSIS_STATE, // chassis_state_t (v_filter, x_filter, pitch, gyro...)
       TOPIC_LEG_STATE,     // leg_state_t (left/right 的 theta, L0, torque_set...)
   } TopicID;
   ```
3. 各任务改为:
   - `INS_task`: publish `TOPIC_INS_DATA`
   - `remote_task`: subscribe `TOPIC_INS_DATA` + `TOPIC_CHASSIS_STATE`, publish `TOPIC_CHASSIS_CMD`
   - `chassis_task`: subscribe `TOPIC_CHASSIS_CMD` + `TOPIC_INS_DATA`, publish `TOPIC_CHASSIS_STATE` + `TOPIC_LEG_STATE`
   - `motor_control_task`: subscribe `TOPIC_LEG_STATE` + `TOPIC_CHASSIS_STATE`
   - `observe_task`: subscribe `TOPIC_LEG_STATE`, publish 到 `TOPIC_CHASSIS_STATE`

4. 逐步删除 extern 声明，改为 subscribe 获取数据

**注意**: message_center 是内存拷贝方式（不是指针），适合 FreeRTOS 多任务环境，天然线程安全。

---

## Step 4: 拆分 chassis_task 为独立模块

**目标**: 把 chassis_task.c 从 300 行上帝函数拆成职责单一的模块。

**现状问题**:
- `chassis_task.c` 混合了: VMC 调用、LQR 计算、腿长 PD、跳跃状态机、离地检测、力矩分配
- 改任何一个功能都要在这个大文件里翻找

**目标目录结构**:
```
User/
├── bsp/                    (现有，小写重命名)
├── modules/
│   ├── motor/              (Step 5 做)
│   ├── imu/                (INS_task 移入)
│   ├── remote/             (remote_control 移入)
│   ├── algorithm/          (现有 Algorithm/ 移入)
│   └── message_center/     (Step 3 新增)
├── app/
│   ├── robot_def.h         (已有)
│   ├── chassis/
│   │   ├── chassis_task.c  (只做调度: subscribe → 调用模块 → publish)
│   │   ├── balance_controller.c  (LQR + 各补偿 PD 计算)
│   │   ├── leg_model.c     (VMC 正运动学 + 逆力矩, 从 VMC_calc 重命名)
│   │   ├── jump_fsm.c      (跳跃状态机, 从 chassis_jump_loop 抽出)
│   │   └── fly_detect.c    (离地检测, 从 chassis_ground_detection 抽出)
│   ├── cmd/
│   │   └── robot_cmd.c     (遥控器数据处理, 从 remote_task 演化)
│   └── observe/
│       └── observe_task.c  (速度观测)
└── lib/                    (现有 Lib/)
```

**拆分顺序**:
1. 先抽 `jump_fsm.c` — 最独立，只需要 vmc 和 leg_set
2. 再抽 `fly_detect.c` — 只需要 vmc 和 INS
3. 再抽 `balance_controller.c` — LQR + PD 计算
4. 最后 `chassis_task.c` 变成纯调度

---

## Step 5: 电机驱动抽象

**目标**: DM4310 和 M3508 统一接口，motor_control_task 简化。

**现状问题**:
- `motor_control_task.c` 直接调用 `mit_ctrl()` 和 `CAN_cmd_chassis()`，硬编码了电机类型
- 用 `systick%2` 做分时发送，不够可靠
- 添加新电机类型需要改动多处

**操作**:
1. 定义统一电机接口:
   ```c
   typedef struct {
       void (*set_torque)(void *motor, float torque);
       void (*send)(void *motor);  // 实际 CAN 发送
       // feedback 在 CAN 中断里自动更新
   } MotorInterface;
   ```
2. DM4310 和 M3508 各自实现该接口
3. `motor_control_task` 改为遍历电机数组，按优先级/时序调用 send
4. CAN 接收回调通过电机 ID 查表分发，不再 switch-case

---

## Step 6: 恢复卡尔曼观测器 + 守护机制

**目标**: 恢复速度融合估计，添加离线检测。

**现状问题**:
- `observe_task.c` 的 KF 融合全部被注释，直接用轮速，打滑时速度估计错误
- 没有任何掉线检测：遥控器断连、电机离线、IMU 异常都不会触发保护

**操作**:

### 6a: 恢复卡尔曼观测器
1. 取消 `observe_task.c` 中被注释的 KF 代码
2. 调参 Q/R 矩阵使融合效果合理
3. 保留直接轮速作为 fallback（KF 发散时切换）

### 6b: 添加 daemon 守护模块
1. 从 basic_framework 移植 `modules/daemon/daemon.c/h`
2. 为每个关键外设注册守护:
   - 遥控器: 超过 100ms 无数据 → 停机
   - DM4310: 超过 20ms 无反馈 → 该电机标记离线
   - M3508: 超过 20ms 无反馈 → 该电机标记离线
   - BMI088: 超过 5ms 无更新 → IMU 异常
3. 任何守护触发 → 所有电机输出清零，进入安全模式
4. 遥控器恢复后可重新启动

---

## 已知 Bug 清单

| # | 严重度 | 描述 | 修复时机 |
|---|--------|------|----------|
| 1 | 🔴 | 左腿 VMC 缺少 d_phi0/dd_L0/dd_theta 滤波和 first_flag 完整初始化 | Step 2 |
| 2 | 🟡 | LQR K 矩阵左右共用 `LQR_K_R, LQR_K_R`，应分别传入或确认对称性 | Step 4 |
| 3 | 🟡 | 卡尔曼观测器被注释，直接用轮速，打滑时失效 | Step 6 |
| 4 | 🟢 | `remote_control.c/h` 存了两份（APP/ 和 Devices/Remote_Control/） | Step 2 |
| 5 | 🟢 | `chassis_task.h` 中 `WHEEL_R` 定义带分号 `0.0675f;` 是语法错误（碰巧被宏展开掩盖） | Step 1 ✅ 已修复 |
| 6 | 🟢 | Controller/ 下的模糊 PID 未使用，可考虑删除或替换简单 PID | Step 4 |

---

## 注意事项

- 每步完成后 `git commit`，保持历史可回溯
- Keil MDK 的 .uvprojx 文件需要手动添加新建/移动的源文件到工程
- 移动文件后 Keil 的 Include Path 也需要同步更新
- `robot_def.h` 中的关节偏移量是每台车独立标定的，换车需要重新测量
- LQR K 矩阵目前是固定值，后续可改为腿长插值（`LQR_K_calc` 函数已预留）
