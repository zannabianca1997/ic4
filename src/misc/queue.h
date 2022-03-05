/**
 * @file queue.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Define a queue
 * @version 0.1
 * @date 2022-03-05
 *
 *
 * @copyright Copyright (c) 2022
 */

#ifndef _QUEUE_H
#define _QUEUE_H

#include <stddef.h> //size_t
#include <stdbool.h> 

enum queue_order_e {
    QUEUE_FIFO,
    QUEUE_LIFO
};

typedef struct queue_s queue_t;

queue_t *queue_new(enum queue_order_e order);
void queue_free(queue_t *queue);
queue_t * queue_copy(queue_t *original, void *copy_data(void const*));

void queue_push(queue_t *q, void *data);
void *queue_pop(queue_t *q);

bool queue_is_empty(queue_t *q);
size_t queue_len(queue_t *q);

#endif // _QUEUE_H