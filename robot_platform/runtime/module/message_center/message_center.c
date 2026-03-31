/**
 * @file    message_center.c
 * @brief   Static-memory pub-sub implementation
 */

#include "message_center.h"

#include <string.h>

static Publisher_t topic_pool[MSG_MAX_TOPICS];
static Subscriber_t sub_pool[MSG_MAX_SUBSCRIBERS];
static uint8_t payload_pool[MSG_MAX_TOTAL_PAYLOAD_BYTES];
static size_t payload_pool_used = 0U;

static int topic_name_is_valid(const char *name)
{
    size_t name_length = 0U;

    if (name == NULL || name[0] == '\0') {
        return 0;
    }

    while (name[name_length] != '\0') {
        if (name_length >= MSG_MAX_TOPIC_NAME) {
            return 0;
        }
        ++name_length;
    }

    return 1;
}

static Publisher_t *find_topic(const char *name)
{
    for (int i = 0; i < MSG_MAX_TOPICS; ++i) {
        if (topic_pool[i].used && strcmp(topic_pool[i].topic_name, name) == 0) {
            return &topic_pool[i];
        }
    }

    return NULL;
}

static size_t count_subscribers(const Publisher_t *pub)
{
    size_t count = 0U;
    const Subscriber_t *iter = pub->first_subs;

    while (iter != NULL) {
        ++count;
        iter = iter->next_subs;
    }

    return count;
}

Publisher_t *PubRegister(char *name, size_t data_len)
{
    Publisher_t *existing_topic = NULL;

    if (!topic_name_is_valid(name) || data_len == 0U) {
        return NULL;
    }

    existing_topic = find_topic(name);
    if (existing_topic != NULL) {
        if (existing_topic->data_len != data_len) {
            return NULL;
        }
        return existing_topic;
    }

    if ((MSG_MAX_TOTAL_PAYLOAD_BYTES - payload_pool_used) < data_len) {
        return NULL;
    }

    for (int i = 0; i < MSG_MAX_TOPICS; ++i) {
        if (!topic_pool[i].used) {
            memset(&topic_pool[i], 0, sizeof(Publisher_t));
            topic_pool[i].used = 1U;
            topic_pool[i].data_len = data_len;
            topic_pool[i].payload_offset = payload_pool_used;
            strncpy(topic_pool[i].topic_name, name, MSG_MAX_TOPIC_NAME);
            topic_pool[i].topic_name[MSG_MAX_TOPIC_NAME] = '\0';
            payload_pool_used += data_len;
            return &topic_pool[i];
        }
    }

    return NULL;
}

Subscriber_t *SubRegister(char *name, size_t data_len)
{
    Publisher_t *pub = PubRegister(name, data_len);

    if (pub == NULL) {
        return NULL;
    }

    for (int i = 0; i < MSG_MAX_SUBSCRIBERS; ++i) {
        if (!sub_pool[i].used) {
            Subscriber_t *new_sub = &sub_pool[i];

            memset(new_sub, 0, sizeof(Subscriber_t));
            new_sub->used = 1U;
            new_sub->owner_pub = pub;
            new_sub->last_read_generation = pub->generation;

            if (pub->first_subs == NULL) {
                pub->first_subs = new_sub;
            } else {
                Subscriber_t *tail = pub->first_subs;
                while (tail->next_subs != NULL) {
                    tail = tail->next_subs;
                }
                tail->next_subs = new_sub;
            }

            return new_sub;
        }
    }

    return NULL;
}

uint8_t PubPushMessage(Publisher_t *pub, void *data_ptr)
{
    if (pub == NULL || data_ptr == NULL) {
        return 0U;
    }

    memcpy(&payload_pool[pub->payload_offset], data_ptr, pub->data_len);
    ++pub->generation;

    return (uint8_t)count_subscribers(pub);
}

uint8_t SubGetMessage(Subscriber_t *sub, void *data_ptr)
{
    Publisher_t *pub = NULL;

    if (sub == NULL || data_ptr == NULL || sub->owner_pub == NULL) {
        return 0U;
    }

    pub = sub->owner_pub;
    if (sub->last_read_generation == pub->generation) {
        return 0U;
    }

    memcpy(data_ptr, &payload_pool[pub->payload_offset], pub->data_len);
    sub->last_read_generation = pub->generation;
    return 1U;
}
