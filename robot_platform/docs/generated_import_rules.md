# Generated Import Rules

`runtime/generated/` 的规则必须从第一天就定死，否则后续边界会重新混乱。

## 允许进入 generated 的内容

1. `CubeMX/CubeMX2` 生成的 `Core`
2. `HAL/CMSIS/Middlewares`
3. 启动文件
4. 链接脚本
5. 自动生成的外设初始化文件

## 禁止进入 generated 的内容

1. 控制算法
2. 业务状态机
3. 任务逻辑主体
4. 自定义设备策略
5. 长期人工维护逻辑

## 当前仓库对应关系

对于 `Chassis`，第二阶段拟导入：

- `Chassis/Core/`
- `Chassis/Drivers/`
- `Chassis/Middlewares/`

## 用户代码处理原则

如果 `CubeMX` 生成文件里已有 `USER CODE BEGIN/END` 区块：

1. 第一阶段先保留
2. 第二阶段逐步把业务逻辑移出 generated
3. generated 最终只保留薄 hook

## 生成覆盖原则

1. `generated` 允许被重新生成覆盖
2. 任何需要人工维护的逻辑必须迁出
3. CI 后续应校验 generated 中是否出现业务新增文件

