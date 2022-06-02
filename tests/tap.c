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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tap.h"

static int expected_tests = NO_PLAN;
static int failed_tests = 0;
static int current_test = 0;
static const char *todo_msg = NULL;

static void default_putc(void *stream, char ch) { putc(ch, (FILE *)stream); }
static void default_putchar(void *stream, char ch) { putchar(ch); }

/**
 * @brief Contain the output function
 */
static void (*putc_f)(void *, char) = &default_putchar;
/**
 * @brief A magic cookie for the output function
 */
static void *cookie = NULL;

void set_output(
    void (*new_putc_f)(void *, char),
    void *new_cookie)
{
    if (new_putc_f)
    {
        putc_f = new_putc_f;
        cookie = new_cookie;
    }
    else
    {
        putc_f = &default_putc;
        cookie = new_cookie ? new_cookie : (void *)stdout;
    }
}

/**
 * @brief Print a char to output
 *
 * @param ch the char to print
 */
static void m_putc(char ch) { (*putc_f)(cookie, ch); }

/**
 * @brief Print a string to output
 *
 * @param s the string to print
 */
static void m_puts(const char *s)
{
    for (; *s; s++)
        m_putc(*s);
}

/**
 * @brief Print a integer number to output
 *
 * @param n the number to print
 */
static void m_putd(int n)
{
    if (n > 9)
        m_putd(n / 10);
    m_putc('0' + n % 10);
}
/**
 * @brief Print a hexadecimal number to output
 *
 * @param n the number to print
 */
static void m_putx(int n)
{
    if (n > 15)
        m_putx(n / 16);
    m_putc((n % 16 < 10) ? ('0' + n % 16) : ('A' - 10 + n % 16));
}

/**
 * @brief Print a string of given maximum lenght to output
 *
 * @param s the string to print
 */
static void m_puts_limited(const char *s, size_t len)
{
    const char *end = s + len;
    for (; *s && s < end; s++)
        m_putc(*s);
}

/**
 * @brief start a diagnostic message
 */
static void diagnostic_start() { m_puts("# "); }

/**
 * @brief End a diagnostic message
 */
static void diagnostic_end() { m_putc('\n'); }

/**
 * @brief Print a number as part of a diagnostic message
 *
 * @param n the number to print
 */
static void diagnostic_putd(int n) { m_putd(n); }

/**
 * @brief Print a hex number as part of a diagnostic message
 *
 * @param n the number to print
 */
static void diagnostic_putx(int n) { m_putx(n); }

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
        while (*line && *line != '\n')
            line++;

        // print the line
        m_puts_limited(msg, (size_t)(line - msg));
        if (*line == '\n')
            m_puts("\n# "); // new diagnostic line

        msg = line + 1; // get ready for next line
    } while (*line);    // stop at the NULL
}

void diagnostic(const char *msg)
{
    diagnostic_start();
    diagnostic_puts(msg);
    diagnostic_end();
}

void plan(int tests, const char *skip_msg)
{
    m_puts("TAP version 14\n");

    expected_tests = tests;
    if (tests == SKIP_ALL)
    {
        m_puts("1..0 ");

        diagnostic_start();
        diagnostic_puts("SKIP ");
        diagnostic_puts(skip_msg);
        diagnostic_end();

        exit(0);
    }
    if (tests != NO_PLAN)
    {
        m_puts("1..");
        m_putd(tests);
        m_putc('\n');
    }
}

int ok_at_loc(const char *file, int line, int test, const char *name)
{
    if (!test)
        m_puts("not ");
    m_puts("ok ");
    m_putd(++current_test);

    if (*name)
    {
        m_puts(" - ");
        m_puts(name);
    }

    if (todo_msg)
    {
        m_puts(" # T"
               "ODO");
        if (*todo_msg)
        {
            m_putc(' ');
            m_puts(todo_msg);
        }
    }
    m_putc('\n');
    if (!test)
    {
        m_puts("#   Failed ");
        if (todo_msg)
            m_puts("(TODO) ");
        m_puts("test ");
        if (*name)
        {
            m_putc('\'');
            m_puts(name);
            m_puts("'\n#  ");
        }

        m_puts("at ");
        m_puts(file);
        m_puts(" line ");
        m_putd(line);
        m_puts(".\n");

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

int exit_status()
{
    int retval = 0;
    if (expected_tests == NO_PLAN)
    {
        m_puts("1..");
        m_putd(current_test);
        m_putc('\n');
    }
    else if (current_test != expected_tests)
    {
        diagnostic_start();
        diagnostic_puts("Looks like you planned ");
        m_putd(expected_tests);
        diagnostic_puts(" test");
        if (expected_tests > 1)
            diagnostic_puts("s");
        diagnostic_puts(" but ran ");
        m_putd(current_test);
        diagnostic_puts(".");
        diagnostic_end();

        retval = 2;
    }
    if (failed_tests)
    {
        diagnostic_start();
        diagnostic_puts("Looks like you failed ");
        m_putd(failed_tests);
        diagnostic_puts(" test");
        if (failed_tests > 1)
            diagnostic_puts("s");
        diagnostic_puts(" of ");
        m_putd(current_test);
        diagnostic_puts(" run.");
        diagnostic_end();

        retval = 1;
    }
    return retval;
}

void bail_out(const char *msg)
{
    m_puts("Bail out! ");
    m_puts(msg);
    m_putc('\n');
    exit(255);
}

void skip(int n, const char *msg)
{
    while (n-- > 0)
    {
        m_puts("ok ");
        m_putd(++current_test);
        m_putc(' ');

        diagnostic_start();
        diagnostic_puts("skip ");
        diagnostic_puts(msg);
        diagnostic_end();
    }
}

void todo(const char *msg) { todo_msg = msg; }
void end_todo() { todo_msg = NULL; }
