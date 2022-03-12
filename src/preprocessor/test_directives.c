/**
 * @file test_directives.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Test the functionalities of directives.c
 * @version 0.1
 * @date 2022-03-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#define _POSIX_C_SOURCE 200809L // needed to use fmemopen
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../misc/bookmark.h"
#include "../misc/log/log.h"
#include "../misc/context/context.h"

#include "lines.h"
#include "linesco.h"

#include "tokenizer.h"
#include "tokenizerco.h"

#include "directives.h"

#include "enum_strings.h"
#include "pp_tok_tostring.h"
#include "../misc/charescape.h"

#define TESTS_MAX_MACRO_ARGS 10
#define TESTS_MAX_MACRO_TOKENS 10
#define TESTS_MAX_DIRECTIVE_ARGS 10

static inline char *format(const char *fmt, ...)
{
    va_list args, args_c;
    va_start(args, fmt);
    va_copy(args_c, args);

    char *buffer = malloc(vsnprintf(NULL, 0, fmt, args_c));
    if (buffer == NULL)
        log_error(NULL, "Malloc failed in allocating string buffer");
    vsprintf(buffer, fmt, args);

    va_end(args);
    va_end(args_c);

    return buffer;
}

/**
 * @brief Contain the data to compare a preprocessor directive against.
 * Very similar to pp_directive_s, except it's pointerless to permit static inizializtion
 *
 */
struct pp_expected_directive_s
{
    enum
    {
        EXPECTED_IGNORE,      // the directive is ignored
        EXPECTED_EXACT,       // the directive must be identical
        EXPECTED_TYPE,        // the directive must have same type
        EXPECTED_CONTENT,     // the directive must have same type and content
        EXPECTED_MARK,        // the directive must be in the same place
        EXPECTED_END,         // the sequence must have finished
        EXPECTED_STOP_COMPARE // do not check further
    } compare_type;

    enum pp_directive_type_e type;
    struct bookmark_s mark;

    union
    {
        struct
        {
            bool need_macros;
            union
            {
                struct
                {
                    size_t line_num;
                    char *file_name;
                };
                struct
                {
                    size_t nargs;
                    struct pp_token_s args[TESTS_MAX_DIRECTIVE_ARGS];
                };
            };
        } line_ctrl;

        struct
        {
            bool need_macros;
            union
            {
                struct
                {
                    char *file_name;
                    bool is_angled;
                };
                struct
                {
                    size_t nargs;
                    struct pp_token_s args[TESTS_MAX_DIRECTIVE_ARGS];
                };
            };
        } include;

        struct
        {
            char *macro_name;
            bool is_function;

            char *args[TESTS_MAX_MACRO_ARGS];
            size_t nargs;

            size_t ntokens;
            struct pp_token_s tokens[TESTS_MAX_MACRO_TOKENS];
        } define;

        struct
        {
            enum loglevel_e severity;
            char *msg;
        } error;

        struct
        {
            size_t nargs;
            struct pp_token_s args[TESTS_MAX_DIRECTIVE_ARGS];
        };
    };
};

/**
 * @brief check a directive against an expected directive
 *
 * @param obtained the directive to check
 * @param expected the expected directive
 * @return char* NULL if successfull, a description of the proplem otherwhise
 */
static char *check_directive(struct pp_directive_s const *obtained, struct pp_expected_directive_s const *expected)
{
    if (expected->compare_type == EXPECTED_IGNORE)
        return NULL;

    if (expected->compare_type == EXPECTED_TYPE)
        return (obtained->type == expected->type) ? NULL : format("Expected %s directive, found %s", directive_name(expected->type), directive_name(obtained->type));

    if (expected->compare_type == EXPECTED_MARK)
        return bookmark_cmp(obtained->mark, expected->mark, CMP_EXACT, CMP_EXACT) ? NULL : format("Expected directive at %lu:%lu, found at %lu:%lu", expected->mark.row, expected->mark.col, obtained->mark.row, obtained->mark.col);

    // remain EXPECTED_EXACT and EXPECTED_CONTENT

    if (expected->compare_type == EXPECTED_EXACT)
        if (!bookmark_cmp(obtained->mark, expected->mark, CMP_EXACT, CMP_EXACT))
            return format("Expected directive at %lu:%lu, found at %lu:%lu", expected->mark.row, expected->mark.col, obtained->mark.row, obtained->mark.col);

    // now we need to check content alone

    if (obtained->type != expected->type)
        return format("Expected %s directive, found %s", directive_name(expected->type), directive_name(obtained->type));

    switch (obtained->type)
    {
    case PP_DIRECTIVE_LINE_CTRL:
        if (obtained->line_ctrl.need_macros != expected->line_ctrl.need_macros)
        {
            if (expected->line_ctrl.need_macros)
                return "Expected a macro-expanded line control, found a normal one";
            else
                return "Expected a normal line control, found a macro expanded one";
        }
        if (obtained->line_ctrl.need_macros)
        {
            if (obtained->line_ctrl.nargs != expected->line_ctrl.nargs)
                return "Different number of arguments to line control";
            for (size_t i = 0; i < obtained->line_ctrl.nargs; i++)
                if (!pp_tok_cmp(obtained->line_ctrl.args[i], &(expected->line_ctrl.args[i])))
                    return format("Linecontrol arg number %lu is different", i);
        }
        else if (obtained->line_ctrl.line_num != expected->line_ctrl.line_num ||
                 strcmp(obtained->line_ctrl.file_name, expected->line_ctrl.file_name) != 0)
            return "Different arguments to line control";

        break;

    case PP_DIRECTIVE_INCLUDE:
        if (obtained->include.need_macros != expected->include.need_macros)
        {
            if (expected->include.need_macros)
                return "Expected a macro-expanded include, found a normal one";
            else
                return "Expected a normal include, found a macro expanded one";
        }
        if (obtained->include.need_macros)
        {
            if (obtained->include.nargs != expected->include.nargs)
                return "Different number of arguments to include";
            for (size_t i = 0; i < obtained->include.nargs; i++)
                if (!pp_tok_cmp(obtained->include.args[i], &(expected->include.args[i])))
                    return format("include arg number %lu is different", i);
        }
        else if (obtained->include.is_angled != expected->include.is_angled ||
                 strcmp(obtained->include.file_name, expected->include.file_name) != 0)
            return "Different arguments to include";
        break;

    case PP_DIRECTIVE_DEFINE:
        if (obtained->define.ntokens != expected->define.ntokens ||
            obtained->define.is_function != expected->define.is_function ||
            strcmp(obtained->define.macro_name, expected->define.macro_name) != 0)
            return "Different macro name or number of tokens in define";

        // checking definition tokens
        for (size_t i = 0; i < obtained->define.ntokens; i++)
            if (!pp_tok_cmp(obtained->define.tokens[i], &(expected->define.tokens[i])))
                return format("macro definition token number %lu is different", i);

        // if function-like, checking args
        if (obtained->define.is_function)
        {
            if (obtained->define.nargs != expected->define.nargs)
                return "Different number of macro arguments";
            for (size_t i = 0; i < obtained->define.nargs; i++)
                if (strcmp(obtained->define.args[i], expected->define.args[i]) != 0)
                    return format("macro arg number %lu is different", i);
        }
        break;

    case PP_DIRECTIVE_ERROR:
        if (obtained->error.severity != expected->error.severity)
            return "Different error severity";
        if (strcmp(obtained->error.msg, expected->error.msg) != 0)
            return "Different error message";
        break;

    // these directive use only generic arguments. No difference in checking...
    case PP_DIRECTIVE_IF:
    case PP_DIRECTIVE_ELIF:
    case PP_DIRECTIVE_PRAGMA:
    case PP_DIRECTIVE_EMIT:
        if (obtained->nargs != expected->nargs)
            return format("Wrong number of arguments: %lu expected, got %lu", obtained->nargs, expected->nargs);
        for (size_t i = 0; i < obtained->nargs; i++)
            if (!pp_tok_cmp(obtained->args[i], &(expected->args[i])))
                return format("Argument number %lu is different: expected %s, found %s", i, pp_tok_tostring(obtained->args[i]), pp_tok_tostring(&(expected->args[i])));
        break;

    // these directive are contentless, no check is required
    case PP_DIRECTIVE_ELSE:
    case PP_DIRECTIVE_ENDIF:
        break;
    }

    return NULL;
}

/**
 * @brief Test if a text matches the given expected directives
 *
 * @param testcase the name of the testcase
 * @param text the text to parse
 * @param exp_directives the directives to match
 * @return char* NULL if matched, or the error encountered
 */
static char *_test_directives(char const *testcase, char const *text, struct pp_expected_directive_s const *exp_directives)
{
    context_t *lcontext = context_new(NULL, testcase);

    // opening the various streams
#pragma GCC diagnostic push
// text is a const char *, but i don't want to copy it in a buffer, and "r" guarantee it will be only read
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    FILE *text_f = fmemopen(text, strlen(text), "r");
#pragma GCC diagnostic pop
    if (text_f == NULL)
        return "Cannot open memory as stream";
    linestream_t *lines = linestream_open(lcontext, text_f);
    if (lines == NULL)
        return "Cannot open linestream";
    pp_tokstream_t *pp_tokstm = pp_tokstream_open(lcontext, lines);
    if (pp_tokstm == NULL)
        return "Cannot open tokenstream";
    directive_stream_t *dirstm = directive_stream_open(lcontext, pp_tokstm);
    if (dirstm == NULL)
        return "Cannot open directive stream";

    for (struct pp_directive_s *directive = directive_stream_get(lcontext, dirstm);
         directive != NULL;
         directive_free(directive), directive = directive_stream_get(lcontext, dirstm), exp_directives++)
    {
        if (exp_directives->compare_type == EXPECTED_END)
            return format("Expected end of directives, obtained %s", directive_name(directive->type));
        if (exp_directives->compare_type == EXPECTED_STOP_COMPARE)
            return NULL; // they all matched

        // compare
        char *check_result = check_directive(directive, exp_directives);
        if (check_result != NULL)
            return check_result;
    }

    if (exp_directives->compare_type != EXPECTED_END && exp_directives->compare_type != EXPECTED_STOP_COMPARE)
        return "Unexpected end of input";

    return NULL;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#define TEST(TESTCASE, TEXT, ...)                                             \
    const char *test_directives_##TESTCASE()                                  \
    {                                                                         \
        static const struct pp_expected_directive_s expected_directives[] = { \
            __VA_ARGS__, {EXPECTED_END}};                                     \
        return _test_directives(#TESTCASE, TEXT, expected_directives);        \
    }

// --- TESTS ---

TEST(running_text,
     "hello, nice \n to meet you!",
     {EXPECTED_CONTENT, PP_DIRECTIVE_EMIT, .nargs = 7, .args = {
                                                           // @formatter:off
                                                           {PP_TOK_IDENTIFIER, .name = "hello"},
                                                           {PP_TOK_PUNCTUATOR, .punc_kind = PUNC_COMMA},
                                                           {PP_TOK_IDENTIFIER, .name = "nice"},
                                                           {PP_TOK_IDENTIFIER, .name = "to"},
                                                           {PP_TOK_IDENTIFIER, .name = "meet"},
                                                           {PP_TOK_IDENTIFIER, .name = "you"},
                                                           {PP_TOK_PUNCTUATOR, .punc_kind = PUNC_NOT}
                                                           // @formatter:on
                                                       }})
TEST(error_delaying,
     "Those @ errors ` will be reported after",
    // @formatter:off
     {EXPECTED_CONTENT, PP_DIRECTIVE_EMIT, .nargs = 6, .args = {
                                                           {PP_TOK_IDENTIFIER, .name = "Those"},
                                                           {PP_TOK_IDENTIFIER, .name = "errors"},
                                                           {PP_TOK_IDENTIFIER, .name = "will"},
                                                           {PP_TOK_IDENTIFIER, .name = "be"},
                                                           {PP_TOK_IDENTIFIER, .name = "reported"},
                                                           {PP_TOK_IDENTIFIER, .name = "after"},
                                                       }},
     {EXPECTED_CONTENT, PP_DIRECTIVE_ERROR, .error = {.severity = LOG_ERROR, .msg = "Stray '@' in input"}}, 
     {EXPECTED_CONTENT, PP_DIRECTIVE_ERROR, .error = {.severity = LOG_ERROR, .msg = "Stray '`' in input"}}
    // @formatter:on
)

#pragma GCC diagnostic pop // "-Wmissing-field-initializers"

#undef TEST