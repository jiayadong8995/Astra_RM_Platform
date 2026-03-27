# Astra Robot Framework (ARF)

轻量级嵌入式机器人控制框架，面向 STM32 + FreeRTOS 环境。

## 核心概念

- **Topic**: 命名的数据通道，固定大小的共享数据槽
- **Node**: 功能模块，通过 publish/subscribe 与其他模块通信
- **零拷贝读**: 订阅者获取数据指针，无需拷贝
- **线程安全**: 写入时中断级自旋保护

## 使用示例

```c
// 定义消息类型
typedef struct { float pitch, roll, yaw; } imu_msg_t;

// 声明 topic
ARF_TOPIC_DEF(imu_data, imu_msg_t);

// 发布者 (INS task)
imu_msg_t *msg = arf_topic_borrow(&imu_data);
msg->pitch = 0.1f;
arf_topic_publish(&imu_data);

// 订阅者 (Chassis task)
const imu_msg_t *imu = arf_topic_read(&imu_data);
if (imu) { /* use imu->pitch ... */ }
```

## 设计原则

1. 零动态内存分配 — 所有 topic 静态定义
2. 最小开销 — 无队列拷贝，读端零拷贝
3. 单写多读 — 每个 topic 一个发布者，多个订阅者
4. 可选时间戳 — 用于检测数据超时
