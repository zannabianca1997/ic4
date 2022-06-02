/**
 * @file tap.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Implement a basic TAP emitter
 * @version 0.1
 * @date 2022-06-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma GCC poison printf sprintf vprintf vsprintf malloc calloc free realloc

#include <stdlib.h>
#include <string.h>

#include "tap.h"

static int expected_tests = NO_PLAN;
static int failed_tests = 0;
static int current_test = 0;
static char *todo_msg = NULL;

/**
 * @brief Contain the output function
 */
static void (*putc_f)(void *, char) = NULL;
/**
 * @brief A magic cookie for the output function
 */
static void *cookie = NULL;

void set_output(
    void (*putc)(void *, char),
    void *cookie)
{
    putc_f = putc;
    cookie = cookie;
}

/**
 * @brief Print a char to output
 *
 * @param ch the char to print
 */
static void putc(char ch) { (*putc_f)(cookie, ch); }

/**
 * @brief Print a string to output
 *
 * @param s the string to print
 */
static void puts(const char *s)
{
    for (; *s; s++)
        putc(*s);
}

/**
 * @brief Print a integer number to output
 *
 * @param n the number to print
 */
static void putd(int n)
{
    if (n > 9)
        putd(n / 10);
    putc('0' + n % 10);
}
/**
 * @brief Print a hexadecimal number to output
 *
 * @param n the number to print
 */
static void putx(int n)
{
    if (n > 15)
        putx(n / 16);
    putc((n % 16 < 10) ? ('0' + n % 16) : ('A' - 10 + n % 16));
}

/**
 * @brief Print a string of given maximum lenght to output
 *
 * @param s the string to print
 */
static void puts_limited(const char *s, size_t len)
{
    const char *end = s + len;
    for (; *s && s < len; s++)
        putc(*s);
}

void plan(int tests, const char *msg)
{
    expected_tests = tests;
    if (tests == SKIP_ALL)
    {
        puts("1..0 ");

        diagnostic_start();
        diagnostic_puts("SKIP ");
        diagnostic_puts(msg);
        diagnostic_end();

        exit(0);
    }
    if (tests != NO_PLAN)
    {
        puts("1..");
        putd(tests);
        putc('\n');
    }
}

int ok_at_loc(const char *file, int line, int test, const char *name)
{
    if (!test)
        puts("not ");
    puts("ok ");
    putd(++current_test);

    if (*name)
    {
        puts(" - ");
        puts(name);
    }

    if (todo_msg)
    {
        puts(" # T"
             "ODO");
        if (*todo_msg)
        {
            putc(' ');
            puts(todo_msg);
        }
    }
    putc('\n');
    if (!test)
    {
        puts("#   Failed ");
        if (todo_msg)
            puts("(TODO) ");
        puts("test ");
        if (*name)
        {
            putc('\'');
            puts(name);
            puts("'\n#  ");
        }

        puts("at ");
        puts(file);
        puts(" line ");
        putd(line);
        puts(".\n");

        if (!todo_msg)
            failed_tests++;
    }
    return test;
}

static int
mystrcmp(const char *a, const char *b)
{
    return a == b ? 0 : !a ? -1
                    : !b   ? 1
                           : strcmp(a, b);
}

#define eq(a, b) (!mystrcmp(a, b))
#define ne(a, b) (mystrcmp(a, b))

int is_at_loc(const char *file, int line, const char *got,
              const char *expected, const char *name)
{
    int test = eq(got, expected);
    ok_at_loc(file, line, test, name);
    if (!test)
    {
        diagnostic_start();
        diagnostic_puts("         got: '");
        diagnostic_puts(got);
        diagnostic_puts("'\n    expected: '");
        diagnostic_puts(expected);
        diagnostic_puts("'");
        diagnostic_end();
    }
    return test;
}

int isnt_at_loc(const char *file, int line, const char *got, const char *expected,
                const char *msg)
{
    int test = ne(got, expected);
    ok_at_loc(file, line, test, msg);
    if (!test)
    {
        diagnostic_start();
        diagnostic_puts("         got: '");
        diagnostic_puts(got);
        diagnostic_puts("'\n    expected: anything else");
        diagnostic_end();
    }
    return test;
}

int cmp_ok_at_loc(const char *file, int line, int a, const char *op, int b,
                  const char *name)
{
    int test = eq(op, "||")   ? a || b
               : eq(op, "&&") ? a && b
               : eq(op, "|")  ? a | b
               : eq(op, "^")  ? a ^ b
               : eq(op, "&")  ? a & b
               : eq(op, "==") ? a == b
               : eq(op, "!=") ? a != b
               : eq(op, "<")  ? a < b
               : eq(op, ">")  ? a > b
               : eq(op, "<=") ? a <= b
               : eq(op, ">=") ? a >= b
               : eq(op, "<<") ? a << b
               : eq(op, ">>") ? a >> b
               : eq(op, "+")  ? a + b
               : eq(op, "-")  ? a - b
               : eq(op, "*")  ? a * b
               : eq(op, "/")  ? a / b
               : eq(op, "%")  ? a % b
                              : (diagnostic_start(),
                                diagnostic_puts("unrecognized operator '"),
                                diagnostic_puts(op),
                                diagnostic_puts("'"),
                                diagnostic_end(),
                                0);
    ok_at_loc(file, line, test, name);
    if (!test)
    {
        diagnostic_start();
        diagnostic_puts("    ");
        diagnostic_putd(a);
        diagnostic_puts("\n        ");
        diagnostic_puts(op);
        diagnostic_puts("\n    ");
        diagnostic_putd(b);
        diagnostic_end();
    }
    return test;
}

/**
 * @brief Check two memory segments for differences
 *
 * @param a the first memory segment start
 * @param b the second memory segment start
 * @param n the lenght of the memory segments
 * @param offset where to store the offset of the first difference
 * @return int 0 if equals, 1 if different, 2 if one of two is invalid
 */
static int find_mem_diff(const char *a, const char *b, size_t n, size_t *offset)
{
    size_t i;
    if (a == b)
        return 0;
    if (!a || !b)
        return 2;
    for (i = 0; i < n; i++)
        if (a[i] != b[i])
        {
            *offset = i;
            return 1;
        }
    return 0;
}

int cmp_mem_at_loc(const char *file, int line, const void *got,
                   const void *expected, size_t n, const char *name)
{
    size_t offset;
    int diff = find_mem_diff(got, expected, n, &offset);
    ok_at_loc(file, line, !diff, name);
    if (diff == 1)
    {
        diagnostic_start();
        diagnostic_puts("    Difference starts at offset ");
        diagnostic_putd(offset);
        diagnostic_puts("\n         got: 0x");
        diagnostic_putx(((unsigned char *)got)[offset]);
        diagnostic_puts("\n    expected: 0x");
        diagnostic_putx(((unsigned char *)expected)[offset]);
        diagnostic_end();
    }
    else if (diff == 2)
    {
        diagnostic_start();
        diagnostic_puts("         got: ");
        diagnostic_puts(got ? "not NULL" : "NULL");
        diagnostic_puts("\n    expected: ");
        diagnostic_puts(expected ? "not NULL" : "NULL");
        diagnostic_end();
    }
    return !diff;
}

/**
 * @brief start a diagnostic message
 */
static void diagnostic_start() { puts("# "); }

/**
 * @brief End a diagnostic message
 */
static void diagnostic_end() { putc('\n'); }

/**
 * @brief Print a number as part of a diagnostic message
 *
 * @param n the number to print
 */
static void diagnostic_putd(int n) { putd(n); }

/**
 * @brief Print a hex number as part of a diagnostic message
 *
 * @param n the number to print
 */
static void diagnostic_putx(int n) { putx(n); }

/**
 * @brief Print part of a diagnostic message
 * @param msg
 */
static void diagnostic_puts(const char *msg)
{
    // we need to find the newline to reprint the diagnostic start

    char const *line; // mark the end of the line

    do
    {
        line = msg;

        // run to the end of the line
        while (*line || *line != '\n')
            line++;

        // print the line
        puts_limited(msg, (size_t)(line - msg));
        if (*line != '\n')
            puts("\n# "); // new diagnostic line

        msg = line + 1; // get ready for next line
    } while (*line);    // stop at the NULL
}

void diagnostic(const char *msg)
{
    diagnostic_start();
    diagnostic_puts(msg);
    diagnostic_end();
}

int exit_status()
{
    int retval = 0;
    if (expected_tests == NO_PLAN)
    {
        puts("1..");
        putd(current_test);
        putc('\n');
    }
    else if (current_test != expected_tests)
    {
        diagnostic_start();
        diagnostic_puts("Looks like you planned ");
        putd(expected_tests);
        diagnostic_puts(" test");
        if (expected_tests > 1)
            diagnostic_puts("s");
        diagnostic_puts(" but ran ");
        putd(current_test);
        diagnostic_puts(".");
        diagnostic_end();

        retval = 2;
    }
    if (failed_tests)
    {
        diagnostic_start();
        diagnostic_puts("Looks like you failed ");
        putd(failed_tests);
        diagnostic_puts(" test");
        if (failed_tests > 1)
            diagnostic_puts("s");
        diagnostic_puts(" of ");
        putd(current_test);
        diagnostic_puts(" run.");
        diagnostic_end();

        retval = 1;
    }
    return retval;
}

void bail_out(const char *msg)
{
    puts("Bail out! ");
    puts(msg);
    putc('\n');
    exit(255);
}

void skip(int n, const char *msg)
{
    while (n-- > 0)
    {
        puts("ok ");
        putd(++current_test);
        putc(' ');

        diagnostic_start();
        diagnostic_puts("skip ");
        diagnostic_puts(msg);
        diagnostic_end();
    }
}

void todo(const char *msg) { todo_msg = msg; }
void end_todo() { todo_msg = NULL; }
