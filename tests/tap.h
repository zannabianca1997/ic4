/**
 * @file tap.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Provide emitters for the TAP protocol
 * @version 0.1
 * @date 2022-06-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _TAP_H
#define _TAP_H

#include <stddef.h>

int ok_at_loc(const char *file, int line, int test, const char *msg);
int is_at_loc(const char *file, int line, const char *got,
              const char *expected, const char *msg);
int isnt_at_loc(const char *file, int line, const char *got,
                const char *expected, const char *msg);
int cmp_ok_at_loc(const char *file, int line, int a, const char *op,
                  int b, const char *msg);
int cmp_mem_at_loc(const char *file, int line, const void *got,
                   const void *expected, size_t n, const char *msg);
int bail_out_if(int test, const char *msg);
void plan(int tests, const char *msg);
int diagnostic(const char *msg);
int exit_status(void);
void skip(int n, const char *msg);
void todo(const char *msg);
void end_todo(void);

#define NO_PLAN -1
#define SKIP_ALL -2
#define ok(test, msg) ok_at_loc(__FILE__, __LINE__, test, msg)
#define is(got, expected, msg) is_at_loc(__FILE__, __LINE__, got, expected, msg)
#define isnt(got, expected, msg) isnt_at_loc(__FILE__, __LINE__, got, expected, msg)
#define cmp_ok(a, op, b, msg) cmp_ok_at_loc(__FILE__, __LINE__, a, op, b, msg)
#define cmp_mem(got, expected, n, msg) cmp_mem_at_loc(__FILE__, __LINE__, got, expected, n, msg)

#define BAIL_OUT(msg) bail_out_if(1, msg)
#define pass(msg) ok(1, msg)
#define fail(msg) ok(0, msg)

#define skip_if(test, n, msg) \
    if (test)                 \
        skip(n, msg);         \
    else

#endif