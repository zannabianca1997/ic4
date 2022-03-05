/**
 * @file queue.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Define a FIFO queue interface
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

/**
 * @brief Contain a FIFO queue of void*
 */
typedef struct queue_s queue_t;

/**
 * @brief Create a new empty queue
 * 
 * @return queue_t* the created queue 
 */
queue_t *queue_new();
/**
 * @brief delete a queue
 * 
 * @param queue the queue to delete
 * @param data_free if not NULL will be called on every remaining element
 */
void queue_free(queue_t *queue, void (*data_free)(void *));
/**
 * @brief Copy a queue
 * 
 * @param original the original queue
 * @param data_copy function to copy datas in the queue. If NULL data pointers will be copied
 * @return queue_t* the new queue
 */
queue_t * queue_copy(queue_t const *original, void *(*data_copy)(void const*));

/**
 * @brief push some data into the queue
 * 
 * @param q the queue
 * @param data the data to push
 */
bool queue_push(queue_t *q, void *data);
/**
 * @brief pop some data from the queue
 * 
 * @param q the queue
 * @return void* the popped data
 */
void *queue_pop(queue_t *q);

/**
 * @brief check if the queue is empty
 * 
 * @param q the queue
 * @return true queue is empty
 * @return false queue is not empty
 */
bool queue_is_empty(queue_t const *q);
/**
 * @brief measure the lenght of the queue
 * 
 * @param q the queue
 * @return size_t the measured len
 */
size_t queue_len(queue_t const *q);

#endif // _QUEUE_H