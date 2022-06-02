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

/**
 * @brief Do a binary check
 *
 * @param file the file that called the test
 * @param line the line at which the test is called
 * @param test the test result
 * @param name the name of this test
 * @return int the result of the test
 */
int ok_at_loc(const char *file, int line, int test, const char *name);

/**
 * @brief Check two string for equality
 *
 * @param file the file that called the test
 * @param line the line at which the test is called
 * @param got the received string
 * @param expected the expected string
 * @param name the name of this test
 * @return int the result of the check
 */
int is_at_loc(const char *file, int line, const char *got,
              const char *expected, const char *name);

/**
 * @brief Check two strings for disequality
 *
 * @param file the file that called the test
 * @param line the line at which the test is called
 * @param got the received string
 * @param expected the not expected string
 * @param name the name of this test
 * @return int the result of the check
 */
int isnt_at_loc(const char *file, int line, const char *got,
                const char *expected, const char *name);

/**
 * @brief Compare two number with a given operation
 *
 * @param file the file that called the test
 * @param line the line at which the test is called
 * @param a the first number to compare
 * @param op the operation to check
 * @param b the second number to compare
 * @param name the name of this test
 * @return int the result of the check
 */
int cmp_ok_at_loc(const char *file, int line, int a, const char *op,
                  int b, const char *name);

/**
 * @brief Compare two memory segment, and report the first difference
 *
 * @param file the file that called the test
 * @param line the line at which the test is called
 * @param got the received memory
 * @param expected the expected memory
 * @param n the lenght of the segments to check
 * @param name the name of this test
 * @return int the result of the check
 */
int cmp_mem_at_loc(const char *file, int line, const void *got,
                   const void *expected, size_t n, const char *name);

/**
 * @brief Set the output functions
 *
 * @param m_putc the function used to output a char
 * @param cookie the magic cookie to give to the function
 */
void set_output(
    void (*m_putc)(void *, char),
    void *cookie);

/**
 * @brief Plan a number of tests
 *
 * @param tests the number of planned test
 * @param skip_msg the optional message to add
 */
void plan(int tests, const char *skip_msg);

/**
 * @brief Print a diagnostic message
 *
 * @param msg The message to print
 */
void diagnostic(const char *msg);
/**
 * @brief Skip a given number of tests
 *
 * @param n the number of test skipping
 * @param msg the skip message
 */
void skip(int n, const char *msg);

/**
 * @brief Start skipping test that would fail anyway
 *
 * @param msg the message on whi they are TODOs
 */
void todo(const char *msg);
/**
 * @brief Stop skipping tests
 *
 */
void end_todo(void);

/**
 * @brief Terminate testing.
 *
 * Print a diagnostic message and number of tests, if necessary
 * @return int 1 if test failed,
 *             2 if wrong number of tests,
 *             0 otherwise
 */
int exit_status(void);
/**
 * @brief Instantly stop testing and close the program.
 *
 * @param msg message to give
 */
void bail_out(const char *msg);

#define NO_PLAN -1
#define SKIP_ALL -2
#define ok(test, name) ok_at_loc(__FILE__, __LINE__, test, name)
#define is(got, expected, name) is_at_loc(__FILE__, __LINE__, got, expected, name)
#define isnt(got, expected, name) isnt_at_loc(__FILE__, __LINE__, got, expected, name)
#define cmp_ok(a, op, b, name) cmp_ok_at_loc(__FILE__, __LINE__, a, op, b, name)
#define cmp_mem(got, expected, n, name) cmp_mem_at_loc(__FILE__, __LINE__, got, expected, n, name)

#define pass(name) ok(1, name)
#define fail(name) ok(0, name)

#define skip_if(test, n, skip_msg) \
    if (test)                      \
        skip(n, skip_msg);         \
    else

#endif