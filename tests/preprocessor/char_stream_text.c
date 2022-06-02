/**
 * @file char_stream_text.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Test char_stream.c with normal input: give a string, check output against hand provided
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
 * @brief Test cases for direct test comparison
 *
 */
static const struct
{
    char const *name;
    char const *input;
    char const *expected;
} test_cases[] = {
    {"plain text", "Hello World", "Hello World"},
    {"multiple rows", "Hello\n World", "Hello\n World"},
    {"escaped newline", "Hello\\\n World", "Hello World"},
    {"alphabet", "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz \t\n0123456789!|&\\+-*/^%.:,;?'\"()[]{}<>*_",
     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz \t\n0123456789!|&\\+-*/^%.:,;?'\"()[]{}<>*_"},
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
    size_t max_output_len = 0;
    for (tests_number_len = 0; test_cases[tests_number_len].name; tests_number_len++)
    {
        size_t out_len = strlen(test_cases[tests_number_len].expected);
        if (max_output_len < out_len)
            max_output_len = out_len;
    }

    max_output_len++;    // ending \0
    max_output_len *= 2; // preparing for more output

    // setting up TAP output
    plan(tests_number_len, NULL);

    // running text tests
    for (size_t i = 0; test_cases[i].name; i++)
    {
        struct char_stream cs;
        const char *input = test_cases[i].input;
        cs_open(&cs, &string_stream, (void *)&input);

        // reading back the whole stream
        char output_buffer[max_output_len];
        size_t j = 0;
        while (j < max_output_len && (output_buffer[j++] = cs_getc(&cs).ch) >= 0)
            ;
        output_buffer[j - 1] = '\0';

        is(output_buffer, test_cases[i].expected, test_cases[i].name);
        if (j == max_output_len)
            diagnostic("Output is not fully represented.");
    }

    return exit_status();
}
