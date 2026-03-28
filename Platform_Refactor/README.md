# Astra Platform Refactor

这个目录保留平台重构过程中的关键设计文档。

收口后，这里只保留仍然有参考价值的内容：

1. 架构目标与迁移边界
2. 市场/方案调研
3. 当前采用的 SITL 路线

## 目录说明

```text
Platform_Refactor/
  README.md
  01_Architecture.md
  02_Migration_Map.md
  04_Market_Research.md
  05_SITL_Roadmap.md
```

## 当前权威文档

- [01_Architecture.md](./01_Architecture.md)
- [02_Migration_Map.md](./02_Migration_Map.md)
- [04_Market_Research.md](./04_Market_Research.md)
- [05_SITL_Roadmap.md](./05_SITL_Roadmap.md)

## 当前判断

基于当前仓库实际情况，第一版平台只建议先覆盖 `Chassis`：

- `Chassis` 已经初步有 `Bsp / Algorithm / Controller / APP` 层次
- `Chassis` 已经在尝试引入 message bus 和重构
- `Gimbal` 仍是 legacy 资产，当前不作为平台主线目标

## 建议阅读顺序

1. [01_Architecture.md](./01_Architecture.md)
2. [02_Migration_Map.md](./02_Migration_Map.md)
3. [04_Market_Research.md](./04_Market_Research.md)
4. [05_SITL_Roadmap.md](./05_SITL_Roadmap.md)
