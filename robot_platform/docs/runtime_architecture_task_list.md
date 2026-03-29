# Runtime Architecture Task List

这份文档只保留当前阶段还值得继续做的事。

## 已完成

1. `device`
   - 已收成 `device_layer + device_profile + device nodes`
2. `control`
   - 已以 contracts 作为正式外部边界
3. `app`
   - 已退回 `bringup + intent + io + config`
4. 构建
   - `hw_elf` 和 `sitl` 已恢复可构建

## 剩余任务

### T1. 继续纯化 `control/internal`

目标：

- 继续压缩内部 legacy 结构
- 让 internal model 更接近 contracts 语义

### T2. 继续收紧 `device` profile

目标：

- 让 backend profile 更工程化
- 让后续新增 backend 的注册方式更明确

### T3. `sim` 接口固化

目标：

- 围绕正式 contracts 固化 `sim` 的输入输出边界
- 不再让 `sim` 依赖 runtime 内部实现

### T4. 文档持续收口

目标：

- 继续删除低价值研究文档和过程文档
- 保持文档和代码口径同步

## 当前禁区

- 不再新增新的 runtime 中间层
- 不再把设备细节暴露回 `app`
- 不再以 legacy task 文件名驱动架构决策
- 不再为兼容旧对象恢复主路径重复结构
