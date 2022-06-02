/**
 * @file char_stream.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Test char_stream.c marking capabilities. Give a string, and search for the first occurence of a char
 * @version 0.1
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <string.h>

#include "../tap.h"

#include "char_stream.h"
#include "../misc/bookmark.h"

/**
 * @brief Test cases for mark checking
 *
 * The symbol + is used to search for
 */

static const struct
{
    char const *name;
    char const *input;
    struct bookmark mark;
} test_cases[] = {
    {"start", "+", {1, 1}},
    {"plain", "       +", {1, 8}},
    {"start second line", "       \n+", {2, 1}},
    {"second line", "       \n  +", {2, 3}},
    {"first logical line", "       \\\n  +", {2, 3}},
    {"double escaped newline", "       \\\n\\\n  +", {3, 3}},
    {"logical first char", "\\\n+", {2, 1}},
    {NULL}};

/**
 * @brief Read a string
 *
 * @param ptr pointer to a pointer to the current position
 * @return int readed char
 */
static int string_stream(void *ptr)
{
    const char **s = (const char **)ptr;
    if (!**s)
        return -1;
    return (int)*((*s)++);
}

int main(int argc, char const *argv[])
{
    // measuring test cases
    size_t tests_number_len;
    for (tests_number_len = 0; test_cases[tests_number_len].name; tests_number_len++)
        ;

    // setting up TAP output
    plan(2 * tests_number_len, NULL);

    // running text tests
    for (size_t i = 0; test_cases[i].name; i++)
    {
        struct char_stream cs;
        const char *input = test_cases[i].input;
        cs_open(&cs, &string_stream, (void *)&input);

        // reading back the stream until we find the char +
        struct bookmark mark = {0, 0};
        struct marked_char ch;
        while ((ch = cs_getc(&cs)).ch >= 0 && ch.ch != '+')
            ;
        mark = ch.mark;

        cmp_ok(mark.row, "==", test_cases[i].mark.row,
               test_cases[i].name);
        cmp_ok(mark.col, "==", test_cases[i].mark.col,
               test_cases[i].name);
    }

    return exit_status();
}