/**
 * @file    message_center.c
 * @brief   Static-memory pub-sub implementation
 */

#include "message_center.h"
#include "string.h"

/* static pools - no malloc needed */
static Publisher_t  topic_pool[MSG_MAX_TOPICS];
static Subscriber_t sub_pool[MSG_MAX_SUBSCRIBERS];

/**
 * @brief  Find existing topic or allocate a new slot
 */
Publisher_t *PubRegister(char *name, uint8_t data_len)
{
    /* search for existing topic */
    for (int i = 0; i < MSG_MAX_TOPICS; i++) {
        if (topic_pool[i].used && strcmp(topic_pool[i].topic_name, name) == 0) {
            return &topic_pool[i];
        }
    }
    /* allocate new topic slot */
    for (int i = 0; i < MSG_MAX_TOPICS; i++) {
        if (!topic_pool[i].used) {
            memset(&topic_pool[i], 0, sizeof(Publisher_t));
            topic_pool[i].used = 1;
            topic_pool[i].data_len = data_len;
            strncpy(topic_pool[i].topic_name, name, MSG_MAX_TOPIC_NAME);
            return &topic_pool[i];
        }
    }
    return (void *)0; // pool exhausted
}

/**
 * @brief  Subscribe to a topic. Creates the topic if it doesn't exist yet.
 */
Subscriber_t *SubRegister(char *name, uint8_t data_len)
{
    Publisher_t *pub = PubRegister(name, data_len);
    if (!pub) return (void *)0;

    /* allocate subscriber slot */
    Subscriber_t *new_sub = (void *)0;
    for (int i = 0; i < MSG_MAX_SUBSCRIBERS; i++) {
        if (!sub_pool[i].used) {
            new_sub = &sub_pool[i];
            memset(new_sub, 0, sizeof(Subscriber_t));
            new_sub->used = 1;
            new_sub->data_len = data_len;
            break;
        }
    }
    if (!new_sub) return (void *)0; // pool exhausted

    /* append to subscriber linked list */
    if (!pub->first_subs) {
        pub->first_subs = new_sub;
    } else {
        Subscriber_t *tail = pub->first_subs;
        while (tail->next_subs)
            tail = tail->next_subs;
        tail->next_subs = new_sub;
    }
    return new_sub;
}

/**
 * @brief  Push message to all subscribers (overwrite semantics, latest wins)
 */
uint8_t PubPushMessage(Publisher_t *pub, void *data_ptr)
{
    uint8_t count = 0;
    Subscriber_t *iter = pub->first_subs;
    while (iter) {
        memcpy(iter->data_buf, data_ptr, pub->data_len);
        iter->has_new = 1;
        count++;
        iter = iter->next_subs;
    }
    return count;
}

/**
 * @brief  Read latest message, clears unread flag
 */
uint8_t SubGetMessage(Subscriber_t *sub, void *data_ptr)
{
    if (!sub->has_new)
        return 0;
    memcpy(data_ptr, sub->data_buf, sub->data_len);
    sub->has_new = 0;
    return 1;
}
