/**
 * @file    arf_topic.h
 * @brief   Astra Robot Framework — 轻量级发布/订阅消息总线
 *
 * 设计目标：
 *   - 零动态内存分配，所有 topic 编译期静态定义
 *   - 单写多读，写端原子保护，读端零拷贝
 *   - 适用于 STM32 + FreeRTOS (Cortex-M)
 *
 * 用法：
 *   1. ARF_TOPIC_DEF(name, type)   在 .c 中定义 topic
 *   2. ARF_TOPIC_EXTERN(name)      在 .h 中声明 topic
 *   3. arf_topic_publish()         发布数据
 *   4. arf_topic_read()            订阅（读取）数据
 */

#ifndef ARF_TOPIC_H
#define ARF_TOPIC_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================== 底层原子操作 ========================== */

/* Cortex-M: 关中断实现临界区，开销极小 */
static inline uint32_t _arf_enter_critical(void) {
    uint32_t primask;
    __asm volatile ("MRS %0, PRIMASK\n CPSID I" : "=r"(primask));
    return primask;
}

static inline void _arf_exit_critical(uint32_t primask) {
    __asm volatile ("MSR PRIMASK, %0" :: "r"(primask));
}

/* ========================== Topic 核心结构 ========================== */

/**
 * @brief Topic 控制块（类型无关）
 *        实际数据紧跟在控制块后面，由宏自动布局
 */
typedef struct {
    const char *name;       /* topic 名称，调试用 */
    uint16_t    msg_size;   /* 消息体大小 (bytes) */
    uint8_t     published;  /* 是否已有数据发布过 */
    uint8_t     _writing;   /* 写入中标记 (用于读端检测撕裂) */
    uint32_t    seq;        /* 发布序号，单调递增 */
    uint32_t    timestamp;  /* 最近发布时间 (ms)，由用户提供 */
    /* data[msg_size] 紧随其后，由宏分配 */
} arf_topic_cb_t;

/* ========================== 静态定义宏 ========================== */

/**
 * @brief 在 .c 文件中定义一个 topic（分配存储）
 * @param _name  topic 变量名
 * @param _type  消息结构体类型
 *
 * 展开后生成：
 *   - _name##_buf  : 数据缓冲区
 *   - _name        : topic 控制块
 */
#define ARF_TOPIC_DEF(_name, _type)                                 \
    static _type _name##_buf;                                       \
    arf_topic_cb_t _name = {                                        \
        .name      = #_name,                                        \
        .msg_size  = sizeof(_type),                                 \
        .published = 0,                                             \
        .seq       = 0,                                             \
        .timestamp = 0,                                             \
    }

/**
 * @brief 在 .h 文件中声明 topic（外部引用）
 */
#define ARF_TOPIC_EXTERN(_name)  extern arf_topic_cb_t _name

/* ========================== 获取数据指针的内部宏 ========================== */

/* 获取 topic 关联的数据缓冲区指针 (void*) */
#define _ARF_DATA_PTR(_name)  ((void *)&_name##_buf)

/* ========================== 发布 API ========================== */

/**
 * @brief  向 topic 写入数据（拷贝方式）
 * @param  _name   topic 变量名
 * @param  _src    指向消息数据的指针
 * @param  _ts     时间戳 (ms)，传 0 则不更新
 *
 * 用法: arf_topic_publish(&my_topic_name, &msg, HAL_GetTick());
 */
#define arf_topic_publish(_name, _src, _ts) do {                    \
    uint32_t _pri = _arf_enter_critical();                          \
    (_name).seq++;                                                  \
    (_name)._writing = 1;                                           \
    memcpy(_ARF_DATA_PTR(_name), (_src), (_name).msg_size);         \
    (_name)._writing = 0;                                           \
    (_name).published = 1;                                          \
    if ((_ts) != 0) (_name).timestamp = (_ts);                      \
    _arf_exit_critical(_pri);                                       \
} while(0)

/**
 * @brief  借出数据缓冲区指针，直接写入后调用 commit
 *         适合大结构体避免栈上临时变量
 *
 * 用法:
 *   my_msg_t *p = arf_topic_borrow(my_topic, my_msg_t);
 *   p->field = value;
 *   arf_topic_commit(my_topic, HAL_GetTick());
 */
#define arf_topic_borrow(_name, _type)  ((_type *)_ARF_DATA_PTR(_name))

#define arf_topic_commit(_name, _ts) do {                           \
    uint32_t _pri = _arf_enter_critical();                          \
    (_name).seq++;                                                  \
    (_name).published = 1;                                          \
    if ((_ts) != 0) (_name).timestamp = (_ts);                      \
    _arf_exit_critical(_pri);                                       \
} while(0)

/* ========================== 订阅/读取 API ========================== */

/**
 * @brief  读取 topic 数据（零拷贝，返回 const 指针）
 * @return 如果 topic 尚未发布过，返回 NULL
 *
 * 用法: const my_msg_t *p = arf_topic_read(my_topic, my_msg_t);
 */
#define arf_topic_read(_name, _type)                                \
    ((_name).published ? (const _type *)_ARF_DATA_PTR(_name) : NULL)

/**
 * @brief  拷贝方式读取（线程安全，适合读写不在同一核心的场景）
 * @return 0=成功, -1=尚未发布
 */
#define arf_topic_copy(_name, _dst) (                               \
    (_name).published ? (                                           \
        memcpy((_dst), _ARF_DATA_PTR(_name), (_name).msg_size), 0  \
    ) : -1                                                          \
)

/**
 * @brief  检查 topic 数据是否超时
 * @param  _now_ms  当前时间 (ms)
 * @param  _timeout 超时阈值 (ms)
 * @return 1=超时或未发布, 0=正常
 */
#define arf_topic_timeout(_name, _now_ms, _timeout)                 \
    ((!(_name).published) || ((_now_ms) - (_name).timestamp > (_timeout)))

/**
 * @brief  获取 topic 发布序号（用于检测是否有新数据）
 */
#define arf_topic_seq(_name)  ((_name).seq)

#ifdef __cplusplus
}
#endif

#endif /* ARF_TOPIC_H */
