/**
 * @file test_queue.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Test the functionalities of queue.c
 * @version 0.1
 * @date 2022-03-06
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <string.h>
#include <stdlib.h>

#include "queue.h"

static void *copy_string(void const *str)
{
    char *new_str = malloc(strlen(str) + 1);
    if (new_str == NULL)
        return NULL;
    strcpy(new_str, str);
    return new_str;
}

#define check_push(q, data, num)                         \
    {                                                    \
        if (!queue_push(q, data))                        \
            return "Cannot push \"" data "\"";           \
        if (queue_len(q) != num)                         \
            return "Queue has wrong number of elements"; \
    }
#define check_pop(q, data, num)                        \
    {                                                  \
        if (strcmp(queue_pop(q), data) != 0)           \
            return "Got data different from" data;     \
        if (queue_len(q) != num)                       \
            return "Queue is not empty after popping"; \
    }

const char *test_single()
{
    queue_t *queue = queue_new();
    if (queue == NULL)
        return "Cannot create queue";

    check_push(queue, "Hello", 1);
    check_pop(queue, "Hello", 0);

    queue_free(queue, NULL);
    return NULL;
}
const char *test_multiples()
{
    queue_t *queue = queue_new();
    if (queue == NULL)
        return "Cannot create queue";

    check_push(queue, "Hello", 1);
    check_push(queue, "Good", 2);
    check_push(queue, "Friend", 3);
    check_pop(queue, "Hello", 2);
    check_pop(queue, "Good", 1);
    check_pop(queue, "Friend", 0);

    queue_free(queue, NULL);
    return NULL;
}
const char *test_copy()
{
    queue_t *queue1 = queue_new();
    if (queue1 == NULL)
        return "Cannot create queue 1";

    char hello_elem[] = "Hello";
    queue_push(queue1, hello_elem);
    queue_push(queue1, "Good");
    queue_push(queue1, "Friend");

    queue_t *queue2 = queue_copy(queue1, &copy_string);

    char *hello_element = queue_pop(queue1);
    hello_element[1] = 'a'; // checking it has been copied
    check_pop(queue1, "Good", 1);
    check_pop(queue1, "Friend", 0);

    check_pop(queue2, "Hello", 2);
    check_pop(queue2, "Good", 1);
    check_pop(queue2, "Friend", 0);

    queue_free(queue1, NULL);
    queue_free(queue2, NULL);
    return NULL;
}
const char *test_default_copy()
{
    queue_t *queue1 = queue_new();
    if (queue1 == NULL)
        return "Cannot create queue 1";

    char hello_elem[] = "Hello";
    queue_push(queue1, hello_elem);
    queue_push(queue1, "Good");
    queue_push(queue1, "Friend");

    queue_t *queue2 = queue_copy(queue1, NULL);

    char *hello_element = queue_pop(queue1);
    hello_element[1] = 'a'; // checking it has not been copied
    check_pop(queue1, "Good", 1);
    check_pop(queue1, "Friend", 0);

    check_pop(queue2, "Hallo", 2); // <- notice the difference
    check_pop(queue2, "Good", 1);
    check_pop(queue2, "Friend", 0);

    queue_free(queue1, NULL);
    queue_free(queue2, NULL);
    return NULL;
}

struct freeable{ bool freed;};
static void free_freeable(void *data){(*(struct freeable*)data).freed = true;}

const char *test_free(){
    struct freeable freeables[] = {{true}, {true}, {true}, {true}, {true}, {true}, {true}, {true}};
    queue_t *queue = queue_new();
    if (queue == NULL)
        return "Cannot create queue";
    for (size_t i = 0; i < sizeof(freeables)/sizeof(freeables[0]); i++)
    queue_push(queue, &freeables[i]);

    // free the filled queue
    queue_free(queue, &free_freeable);

    for (size_t i = 0; i < sizeof(freeables)/sizeof(freeables[0]); i++)
        if(!freeables[i].freed)
        return "Not all elements has been freed";
    
    return NULL;
}


#undef push
#undef pop