/**
 * @file queue.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Implement a FIFO queue
 * @version 0.1
 * @date 2022-03-05
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdlib.h>

#include "queue.h"

struct queue_node_s
{
    void *data;
    struct queue_node_s *next;
};

struct queue_s
{
    struct queue_node_s *head;
    struct queue_node_s *tail;
    size_t len;
};

queue_t *queue_new()
{
    struct queue_s *queue_ptr = malloc(sizeof(struct queue_s));
    if (queue_ptr == NULL)
        return NULL;
    queue_ptr->head = NULL;
    queue_ptr->tail = NULL;
    queue_ptr->len = 0;
    return queue_ptr;
}

void queue_free(queue_t *queue, void (*data_free)(void *))
{
    if (data_free != NULL)
        while (!queue_is_empty(queue))
            (*data_free)(queue_pop(queue));
    free(queue);
}

queue_t *queue_copy(queue_t const *original, void *(*data_copy)(void const *))
{

    struct queue_s *queue_ptr = queue_new();
    if (queue_ptr == NULL)
        return NULL;
    for (struct queue_node_s const *data_node = original->head; data_node != NULL; data_node = data_node->next)
        if (data_copy != NULL)
            queue_push(queue_ptr, (*data_copy)(data_node->data));
        else
            queue_push(queue_ptr, data_node->data);
    return queue_ptr;
}

bool queue_push(queue_t *q, void *data)
{
    // create the new data node
    struct queue_node_s *data_node = malloc(sizeof(struct queue_node_s));
    if (data_node == NULL)
        return false;
    data_node->data = data;
    data_node->next = NULL;

    // add node as a tail
    if (q->head == NULL)
        q->head = data_node;
    else
        q->tail->next = data_node;
    q->tail = data_node;
    q->len++;

    return true;
}

void *queue_pop(queue_t *q)
{
    if (queue_is_empty(q))
        return NULL;
    struct queue_node_s *data_node = q->head;
    q->head = q->head->next;
    q->len--;

    void *data = data_node->data;
    free(data_node);
    return data;
}

bool queue_is_empty(queue_t const *q) { return q->head == NULL; }
size_t queue_len(queue_t const *q) { return q->len; }