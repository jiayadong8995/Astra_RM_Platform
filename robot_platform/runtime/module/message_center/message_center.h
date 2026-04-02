/**
 * @file    message_center.h
 * @brief   Lightweight publish-subscribe message center (static memory, zero malloc)
 *          Reference: HNUYueLuRM basic_framework, adapted for static allocation
 *
 * Usage:
 *   // Publisher (e.g. INS_task)
 *   Publisher_t *ins_pub = PubRegister("ins_data", sizeof(platform_ins_state_message_t));
 *   PubPushMessage(ins_pub, &ins_data);
 *
 *   // Subscriber (e.g. chassis_task)
 *   Subscriber_t *ins_sub = SubRegister("ins_data", sizeof(platform_ins_state_message_t));
 *   platform_ins_state_message_t local_ins;
 *   if (SubGetMessage(ins_sub, &local_ins)) { ... }
 */

#ifndef MESSAGE_CENTER_H
#define MESSAGE_CENTER_H

#include <stddef.h>
#include <stdint.h>

/* ============ Capacity config ============ */
#define MSG_MAX_TOPIC_NAME  24
#define MSG_MAX_TOPICS      8
#define MSG_MAX_SUBSCRIBERS 16
#define MSG_MAX_TOTAL_PAYLOAD_BYTES 4096U

/* ============ Subscriber ============ */
struct pub;

typedef struct sub {
    uint8_t used;
    uint32_t last_read_generation;
    struct pub *owner_pub;
    struct sub *next_subs;
} Subscriber_t;

/* ============ Publisher (i.e. topic node) ============ */
typedef struct pub {
    char topic_name[MSG_MAX_TOPIC_NAME + 1];
    size_t data_len;
    size_t payload_offset;
    uint32_t generation;
    uint8_t used;
    uint8_t write_index;
    Subscriber_t *first_subs;
} Publisher_t;

/**
 * @brief  Register as a topic publisher. Duplicate name returns existing instance.
 * @param  name     topic name string
 * @param  data_len data size in bytes (use sizeof)
 * @return publisher pointer, NULL if topic pool exhausted
 */
Publisher_t *PubRegister(char *name, size_t data_len);

/**
 * @brief  Subscribe to a topic
 * @param  name     topic name (must match publisher)
 * @param  data_len data size (must match publisher)
 * @return subscriber pointer, NULL if subscriber pool exhausted
 */
Subscriber_t *SubRegister(char *name, size_t data_len);

/**
 * @brief  Publish message to all subscribers (memcpy overwrite)
 * @return number of subscribers notified
 */
uint8_t PubPushMessage(Publisher_t *pub, void *data_ptr);

/**
 * @brief  Get latest message from subscription
 * @return 1 = new message retrieved, 0 = no new message
 */
uint8_t SubGetMessage(Subscriber_t *sub, void *data_ptr);

#endif // MESSAGE_CENTER_H
