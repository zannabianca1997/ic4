/**
 * @file stack_trace.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Give some basic tracing utilities
 * @version 0.1
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */

/**
 * @brief Contain a stack trace.
 *
 * Need to be saved on stack.
 */
struct stack_trace
{
    const char *const _level_name;
    const struct stack_trace *_next;
};