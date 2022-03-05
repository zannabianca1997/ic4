/**
 * @file test_tokenizer_n.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Test the tokenizer.
 * @version 0.1
 * @date 2022-02-11
 *
 * Given all the tokenizer test are basically a repeat of input a string -> tokenize -> check the result
 *  the test cases are macro generated
 *
 * @copyright Copyright (c) 2022
 *
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lines.h"
#include "linesco.h"

#include "tokenizer.h"
#include "tokenizerco.h"

#include "enum_strings.h"
#include "../misc/charescape.h"

/**
 * @brief Elaborate a human readable token representation
 *
 * @param tok the token to represent
 * @return char* the representation
 */
static char *pp_tok_tostring(struct pp_token_s *tok)
{
    char *result;

    switch (tok->type)
    {
    case PP_TOK_IDENTIFIER:
    case PP_TOK_PP_NUMBER:
        result = malloc(strlen(tok->name) + 1);
        if (result == NULL)
            return "Malloc failed during token string representation";
        strcpy(result, tok->name);
        return result;

    case PP_TOK_STRING_LIT:
        result = malloc(escaped_len(tok->string.value, tok->string.len) + 3);
        if (result == NULL)
            return "Malloc failed during token string representation";
        result[0] = '\"';
        escaped_string(&result[1], tok->string.value, tok->string.len);
        strcat(result, "\"");
        return result;

    case PP_TOK_CHAR_CONST:
        result = malloc(CHARESCAPE_LEN(tok->char_value) + 3);
        if (result == NULL)
            return "Malloc failed during token string representation";
        strcpy(result, "\'");
        strcat(result, CHARESCAPE(tok->char_value));
        strcat(result, "\'");
        return result;

    case PP_TOK_HEADER:
        result = malloc(strlen(tok->header.name) + 3);
        if (result == NULL)
            return "Malloc failed during token string representation";
        strcpy(result, tok->header.is_angled ? "<" : "\"");
        strcat(result, tok->header.name);
        strcat(result, tok->header.is_angled ? ">" : "\"");
        return result;

    case PP_TOK_PUNCTUATOR:;
        char const *name_fmt = "punctuator \"%s\"";
        char *punc_name = malloc(snprintf(NULL, 0, name_fmt, pp_punc_kind_name(tok->punc_kind)));
        if (punc_name == NULL)
            return "Malloc failed during token string representation";
        sprintf(punc_name, name_fmt, pp_punc_kind_name(tok->punc_kind));
        return punc_name;

    case PP_TOK_DIRECTIVE_START:
    case PP_TOK_DIRECTIVE_STOP:;
        const char *msg = (tok->type == PP_TOK_DIRECTIVE_START) ? "directive start" : "directive stop";
        result = malloc(strlen(msg) + 1);
        strcpy(result, msg);
        return result;

    case PP_TOK_ERROR:;
        char const *err_fmt = "%s \"%s\"";
        result = malloc(snprintf(NULL, 0, err_fmt, LOG_LEVEL_NAME[tok->error.severity], tok->error.msg));
        if (result == NULL)
            return "Malloc failed during token string representation";
        sprintf(result, err_fmt, LOG_LEVEL_NAME[tok->error.severity], tok->error.msg);
        return result;

    default:
        return "token of unknow type";
    }
}

/**
 * @brief Describe a token we expect from the tokenizer
 *
 */
struct expected_pp_token_s
{
    enum
    {
        EXPECTED_IGNORE,      // the token is ignored
        EXPECTED_EXACT,       // the token must be identical
        EXPECTED_TYPE,        // the token must have same type
        EXPECTED_CONTENT,     // the token must have same type and content
        EXPECTED_MARK,        // the token must be in the same place
        EXPECTED_END,         // the sequence must have finished
        EXPECTED_STOP_COMPARE // do not check further
    } compare_type;
    struct pp_token_s expected;
};

/**
 * @brief Check if a token has the right type
 *
 * @param obtained the obtained token
 * @param expected the expected one
 * @return const char* NULL if the type is OK or a description of the problem
 */
#ifdef __GNUC__
__attribute_const__
#endif
    static inline const char *
    check_type(struct pp_token_s *obtained, struct pp_token_s *expected)
{
    if (obtained->type == expected->type)
        return NULL;
    char const *MSG_FMT = "Expected token of type \"%s\", obtained instead \"%s\"";
    char *msg = malloc(strlen(MSG_FMT) - 4 + 2 * PP_TYPE_NAME_MAX_LEN + 1);
    if (msg == NULL)
        return "Malloc failed in error message allocation";
    sprintf(msg, MSG_FMT, pp_type_name(expected->type), pp_type_name(obtained->type));
    return msg;
}

/**
 * @brief Check if a token has the right content
 *
 * @param obtained the obtained token
 * @param expected the expected one
 * @return const char* NULL if the content is OK or a description of the problem
 */
#ifdef __GNUC__
__attribute_const__
#endif
    static inline const char *
    check_content(struct pp_token_s *obtained, struct pp_token_s *expected)
{
    if (check_type(obtained, expected) != NULL)
        return check_type(obtained, expected);
    switch (obtained->type)
    {
    case PP_TOK_IDENTIFIER:
    case PP_TOK_PP_NUMBER:
        if (strcmp(obtained->name, expected->name) != 0)
        {
            char const *MSG_FMT = "Expected %s \"%s\", obtained instead \"%s\"";
            char *msg = malloc(strlen(MSG_FMT) - 6 + PP_TYPE_NAME_MAX_LEN + strlen(obtained->name) + strlen(expected->name) + 1);
            if (msg == NULL)
                return "Malloc failed in error message allocation";
            sprintf(msg, MSG_FMT, pp_type_name(obtained->type), expected->name, obtained->name);
            return msg;
        }
        break;
    case PP_TOK_STRING_LIT:
        if (obtained->string.len != expected->string.len || memcmp(obtained->string.value, expected->string.value, obtained->string.len) != 0)
        {
            char const *MSG_1 = "Expected string literal \"";
            char const *MSG_2 = "\", obtained instead \"";
            char const *MSG_3 = "\"";
            char *msg = malloc(strlen(MSG_1) + strlen(MSG_2) + strlen(MSG_3) +
                               escaped_len(obtained->string.value, obtained->string.len) +
                               escaped_len(expected->string.value, expected->string.len) + 1);
            if (msg == NULL)
                return "Malloc failed in error message allocation";
            // creating message
            char *msg_cur = msg;
            strcpy(msg_cur, MSG_1);
            msg_cur += strlen(msg_cur);
            msg_cur = escaped_string(msg_cur, expected->string.value, expected->string.len);
            strcat(msg, MSG_2);
            msg_cur += strlen(msg_cur);
            msg_cur = escaped_string(msg_cur, obtained->string.value, obtained->string.len);
            strcat(msg, MSG_3);

            return msg;
        }
        break;

    case PP_TOK_CHAR_CONST:
        if (obtained->char_value != expected->char_value)
        {
            char const *MSG_FMT = "Expected '%s', obtained instead '%s'";
            char *msg = malloc(strlen(MSG_FMT) - 4 + CHARESCAPE_LEN(obtained->char_value) + CHARESCAPE_LEN(expected->char_value) + 1);
            if (msg == NULL)
                return "Malloc failed in error message allocation";
            sprintf(msg, MSG_FMT, CHARESCAPE(expected->char_value), CHARESCAPE(obtained->char_value));
            return msg;
        }
        break;
    case PP_TOK_HEADER:
        if (expected->header.is_angled && !obtained->header.is_angled)
            return "An angled header was expected, a quoted one was obtained";
        if (!expected->header.is_angled && obtained->header.is_angled)
            return "A quoted header was expected, an angled one was obtained";

        if (strcmp(obtained->header.name, expected->header.name) != 0)
        {
            char const *MSG_FMT = "Expected file name \"%s\", obtained instead \"%s\"";
            char *msg = malloc(strlen(MSG_FMT) - 4 +
                               strlen(obtained->header.name) +
                               strlen(expected->header.name) + 1);
            if (msg == NULL)
                return "Malloc failed in error message allocation";
            sprintf(msg, MSG_FMT, expected->header.name, obtained->header.name);
            return msg;
        }
        break;
    case PP_TOK_PUNCTUATOR:
        if (expected->punc_kind != obtained->punc_kind)
        {
            char const *MSG_FMT = "Expected punctuator of kind \"%s\", obtained instead \"%s\"";
            char *msg = malloc(strlen(MSG_FMT) - 4 + 2 * PP_PUNC_KIND_NAME_MAX_LEN + 1);
            if (msg == NULL)
                return "Malloc failed in error message allocation";
            sprintf(msg, MSG_FMT, pp_punc_kind_name(expected->punc_kind), pp_punc_kind_name(obtained->punc_kind));
            return msg;
        }
        break;

    case PP_TOK_DIRECTIVE_START:
    case PP_TOK_DIRECTIVE_STOP:
        break;

    case PP_TOK_ERROR:
        if (expected->error.severity != obtained->error.severity)
        {
            char const *MSG_FMT = "Expected error of severity \"%s\", obtained instead \"%s\"";
            char *msg = malloc(strlen(MSG_FMT) - 4 + 2 * LOG_LEVEL_NAME_MAX_LEN + 1);
            if (msg == NULL)
                return "Malloc failed in error message allocation";
            sprintf(msg, MSG_FMT, LOG_LEVEL_NAME[expected->error.severity], LOG_LEVEL_NAME[obtained->error.severity]);
            return msg;
        }
        if (strcmp(obtained->error.msg, expected->error.msg) != 0)
        {
            char const *MSG_FMT = "Expected error message \"%s\", obtained instead \"%s\"";
            char *msg = malloc(strlen(MSG_FMT) - 4 +
                               strlen(obtained->error.msg) +
                               strlen(expected->error.msg) + 1);
            if (msg == NULL)
                return "Malloc failed in error message allocation";
            sprintf(msg, MSG_FMT, expected->error.msg, obtained->error.msg);
            return msg;
        }
        break;
    default:
        return "Obtained token of unknow type";
    }

    return NULL;
}

/**
 * @brief Check if a token has the right mark
 *
 * @param obtained the obtained token
 * @param expected the expected one
 * @return const char* NULL if the mark is OK or a description of the problem
 */
#ifdef __GNUC__
__attribute_const__
#endif
    static inline const char *
    check_mark(struct pp_token_s *obtained, struct pp_token_s *expected)
{
    if (!bookmark_cmp(obtained->mark, expected->mark, CMP_EXACT, CMP_EXACT))
    {
        char const *MSG_FMT = "Token %s was expected at %lu:%lu, obtained instead at %lu:%lu";
        char const *tok_str = pp_tok_tostring(obtained);
        char *msg = malloc(snprintf(NULL, 0, MSG_FMT, tok_str, expected->mark.row, expected->mark.col, obtained->mark.row, obtained->mark.col));
        if (msg == NULL)
            return "Malloc failed in error message allocation";
        sprintf(msg, MSG_FMT, tok_str, expected->mark.row, expected->mark.col, obtained->mark.row, obtained->mark.col);
        return msg;
    }
    return NULL;
}

/**
 * @brief Run a test case
 *
 * @param testcase the name of the tes
 * @param text the text to parse
 * @param tokens the expected tokens
 * @param num_tokens the num of tokens to check
 * @return const char*
 */
static const char *_test_tokenize(char const *testcase, char const *text, struct expected_pp_token_s *tokens)
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

    for (
        struct pp_token_s *tok = pp_tokstream_get(lcontext, pp_tokstm);
        tok != NULL;
        pp_tok_free(tok), tok = pp_tokstream_get(lcontext, pp_tokstm), tokens++) // TODO: check we do not exceed number of tokens
        // check how we should compare
        switch (tokens->compare_type)
        {
        case EXPECTED_IGNORE:
            break; // always continue

        case EXPECTED_EXACT:
            if (check_content(tok, &tokens->expected) != NULL)
                return check_content(tok, &tokens->expected);
            if (check_mark(tok, &tokens->expected) != NULL)
                return check_mark(tok, &tokens->expected);
            break;

        case EXPECTED_TYPE:
            if (check_type(tok, &tokens->expected) != NULL)
                return check_type(tok, &tokens->expected);
            break;

        case EXPECTED_CONTENT:
            if (check_content(tok, &tokens->expected) != NULL)
                return check_content(tok, &tokens->expected);
            break;

        case EXPECTED_MARK:
            if (check_mark(tok, &tokens->expected) != NULL)
                return check_mark(tok, &tokens->expected);
            break;

        case EXPECTED_END:;
            // token in excess
            const char *msg_fmt = "Expected end of tokens, found instead %s";
            const char *tok_str = pp_tok_tostring(tok);
            char *msg = malloc(snprintf(NULL, 0, msg_fmt, tok_str));
            if (msg == NULL)
                return "Malloc failed in allocating error message";
            sprintf(msg, msg_fmt, tok_str);
            return msg;

        case EXPECTED_STOP_COMPARE:
            return NULL;
        }

    if (tokens->compare_type != EXPECTED_END && tokens->compare_type != EXPECTED_STOP_COMPARE)
    {
        // tokens in defect
        const char *msg_fmt = "Expected %s, found instead end of tokens";
        const char *tok_str = pp_tok_tostring(&(tokens->expected));
        char *msg = malloc(snprintf(NULL, 0, msg_fmt, tok_str));
        if (msg == NULL)
            return "Malloc failed in allocating error message";
        sprintf(msg, msg_fmt, tok_str);
        return msg;
    }

    return NULL;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#define TEST(TESTCASE, TEXT, ...)                                \
    const char *test_tokenize_##TESTCASE()                       \
    {                                                            \
        struct expected_pp_token_s expected_tokens[] = {         \
            __VA_ARGS__, {EXPECTED_END}};                        \
        return _test_tokenize(#TESTCASE, TEXT, expected_tokens); \
    }

// --- TESTS ---

// -- whitespaces

TEST(space,
     " ", {EXPECTED_END})
TEST(tab,
     "\t", {EXPECTED_END})
TEST(empty_line,
     "\t   \t \t     \t", {EXPECTED_END})
TEST(newline,
     "\n", {EXPECTED_END})
TEST(empty_lines,
     "\t   \t \n\t     \t", {EXPECTED_END})
TEST(mixed_ws,
     "\n\t   \n  \r   \r\n\t \t \v", {EXPECTED_END})
// --- identifiers

TEST(identifier,
     "hello",
     {EXPECTED_CONTENT, {PP_TOK_IDENTIFIER, .name = "hello"}})
TEST(num_id,
     "by42",
     {EXPECTED_CONTENT, {PP_TOK_IDENTIFIER, .name = "by42"}})
TEST(underscore_id,
     "_contain_under___scores",
     {EXPECTED_CONTENT, {PP_TOK_IDENTIFIER, .name = "_contain_under___scores"}})
TEST(identifiers_marking,
     "  foo\nbar32 \\\n baz_hlle ans \t _bara",
     {EXPECTED_EXACT, {PP_TOK_IDENTIFIER, {NULL, 1, 3}, .name = "foo"}},
     {EXPECTED_EXACT, {PP_TOK_IDENTIFIER, {NULL, 2, 1}, .name = "bar32"}},
     {EXPECTED_EXACT, {PP_TOK_IDENTIFIER, {NULL, 3, 2}, .name = "baz_hlle"}}, // even if the newline is escaped, it is still detected
     {EXPECTED_EXACT, {PP_TOK_IDENTIFIER, {NULL, 3, 11}, .name = "ans"}},
     {EXPECTED_EXACT, {PP_TOK_IDENTIFIER, {NULL, 3, 17}, .name = "_bara"}})

// -- preprocessor numbers

TEST(ints,
     "1 23 \n 42 456 22 \n789203\n 3",
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 1, 1}, .name = "1"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 1, 3}, .name = "23"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 2}, .name = "42"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 5}, .name = "456"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 9}, .name = "22"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 3, 1}, .name = "789203"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 4, 2}, .name = "3"}})
TEST(floats,
     "1. 2.3 \n 42. 4.56 2.2 \n78.9203\n 3.",
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 1, 1}, .name = "1."}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 1, 4}, .name = "2.3"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 2}, .name = "42."}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 6}, .name = "4.56"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 11}, .name = "2.2"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 3, 1}, .name = "78.9203"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 4, 2}, .name = "3."}})
TEST(leading_dot,
     ".1 .23 \n .42 .456 .22 \n.789203\n .3",
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 1, 1}, .name = ".1"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 1, 4}, .name = ".23"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 2}, .name = ".42"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 6}, .name = ".456"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 11}, .name = ".22"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 3, 1}, .name = ".789203"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 4, 2}, .name = ".3"}})
TEST(exponents,
     "1.e5 2.3e+5 \n 42e-23 .46E+34 2.2E-1 \n.789203p2\n 3.p+86",
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 1, 1}, .name = "1.e5"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 1, 6}, .name = "2.3e+5"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 2}, .name = "42e-23"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 9}, .name = ".46E+34"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 17}, .name = "2.2E-1"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 3, 1}, .name = ".789203p2"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 4, 2}, .name = "3.p+86"}})
TEST(strange_pp_numbers,
     "1.e849dh34h7...34c.c34.c05 2.3...exb5 \n 0d.e.a.d.b.e.e.f .4..6E 1ex",
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 1, 1}, .name = "1.e849dh34h7...34c.c34.c05"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 1, 28}, .name = "2.3...exb5"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 2}, .name = "0d.e.a.d.b.e.e.f"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 19}, .name = ".4..6E"}},
     {EXPECTED_EXACT, {PP_TOK_PP_NUMBER, {NULL, 2, 26}, .name = "1ex"}})
/*
    This exemplify how the preprocessor can mislead.
    Instead of the valid c code 0xE + 12, it is parsed as a single invalid number
*/
TEST(misleading_parse,
     "0xE+12",
     {EXPECTED_CONTENT, {PP_TOK_PP_NUMBER, .name = "0xE+12"}})

// -- number and id interaction
TEST(number_seems_id,
     "0array",
     {EXPECTED_CONTENT, {PP_TOK_PP_NUMBER, .name = "0array"}})
TEST(id_seems_number,
     "x123456",
     {EXPECTED_CONTENT, {PP_TOK_IDENTIFIER, .name = "x123456"}})

// -- string literals

#define TEST_STR(TESTCASE, STR) \
    TEST(TESTCASE, #STR, {EXPECTED_CONTENT, {PP_TOK_STRING_LIT, .string = {STR, sizeof(STR)}}})

TEST_STR(strlit_so_long, "So long, and thanks for all the fish")
TEST_STR(strlit_escape_chars, "\t \n \r \" \' \\")
TEST_STR(strlit_escape_octals, "\1 \3 \7m \23 \42 \145 \323 \0104")
TEST_STR(strlit_escape_hex, "\xa \xba \xfam \x30")
TEST_STR(strlit_nul, " \0 ")
TEST(strlit_octal_end,
     "\"\\1\" \"\\01\" \"\\001\" \"\\0012\"",
     {EXPECTED_CONTENT, {PP_TOK_STRING_LIT, .string = {"\1", 2}}},
     {EXPECTED_CONTENT, {PP_TOK_STRING_LIT, .string = {"\1", 2}}},
     {EXPECTED_CONTENT, {PP_TOK_STRING_LIT, .string = {"\1", 2}}},
     {EXPECTED_CONTENT, {PP_TOK_STRING_LIT, .string = {"\0012", 3}}})
TEST(strlit_identifier,
     "\"this is a string\"this_is_a_id",
     {EXPECTED_CONTENT, {PP_TOK_STRING_LIT, .string = {"this is a string", 17}}},
     {EXPECTED_CONTENT, {PP_TOK_IDENTIFIER, .name = "this_is_a_id"}})
TEST(strlit_newline,
     "\"this is a string\ncutted by a newline\"",
     {EXPECTED_EXACT, {PP_TOK_ERROR, {NULL, 1, 1}, .error = {.severity = LOG_ERROR, .msg = "Unexpected newline while scanning quoted literal"}}},
     {EXPECTED_CONTENT,{PP_TOK_IDENTIFIER, .name="cutted"}},
     {EXPECTED_CONTENT,{PP_TOK_IDENTIFIER, .name="by"}},
     {EXPECTED_CONTENT,{PP_TOK_IDENTIFIER, .name="a"}},
     {EXPECTED_CONTENT,{PP_TOK_IDENTIFIER, .name="newline"}},
     {EXPECTED_EXACT, {PP_TOK_ERROR, {NULL, 2, 20}, .error = {.severity = LOG_ERROR, .msg = "Unexpected newline while scanning quoted literal"}}})


// -- char consts

TEST(chcon_letters,
     "'a' 'b' 'c'",
     {EXPECTED_EXACT, {PP_TOK_CHAR_CONST, {NULL, 1,1}, .char_value='a'}},
     {EXPECTED_EXACT, {PP_TOK_CHAR_CONST, {NULL, 1,5}, .char_value='b'}},
     {EXPECTED_EXACT, {PP_TOK_CHAR_CONST, {NULL, 1,9}, .char_value='c'}})
TEST(chcon_escaped,
     "'\\42' '\\xa3' '\\n'",
     {EXPECTED_EXACT, {PP_TOK_CHAR_CONST, {NULL, 1,1}, .char_value='\42'}},
     {EXPECTED_EXACT, {PP_TOK_CHAR_CONST, {NULL, 1,7}, .char_value='\xa3'}},
     {EXPECTED_EXACT, {PP_TOK_CHAR_CONST, {NULL, 1,14}, .char_value='\n'}})
TEST(chcon_zero,
     "'\\0'",
     {EXPECTED_EXACT, {PP_TOK_CHAR_CONST, {NULL, 1,1}, .char_value='\0'}})
TEST(chcon_newline,
     "'\n'",
     {EXPECTED_EXACT, {PP_TOK_ERROR, {NULL, 1, 1}, .error = {.severity = LOG_ERROR, .msg = "Unexpected newline while scanning quoted literal"}}},
     {EXPECTED_EXACT, {PP_TOK_ERROR, {NULL, 2, 1}, .error = {.severity = LOG_ERROR, .msg = "Unexpected newline while scanning quoted literal"}}})
#if 0

// -- Punctuators
TEST(punctuators_aritmetics,
     "+ - / * %",
     "<tokens>"
     "<token kind=\"add\" type=\"punctuator\" />"
     "<token kind=\"subtract\" type=\"punctuator\" />"
     "<token kind=\"divide\" type=\"punctuator\" />"
     "<token kind=\"multiply or dereference\" type=\"punctuator\" />"
     "<token kind=\"modulus\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_comparators,
     "== != < <= > >=",
     "<tokens>"
     "<token kind=\"equal\" type=\"punctuator\" />"
     "<token kind=\"not equal\" type=\"punctuator\" />"
     "<token kind=\"less\" type=\"punctuator\" />"
     "<token kind=\"less or equal\" type=\"punctuator\" />"
     "<token kind=\"greather\" type=\"punctuator\" />"
     "<token kind=\"greather or equal\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_logical,
     "! && || ? :",
     "<tokens>"
     "<token kind=\"not\" type=\"punctuator\" />"
     "<token kind=\"and\" type=\"punctuator\" />"
     "<token kind=\"or\" type=\"punctuator\" />"
     "<token kind=\"question mark\" type=\"punctuator\" />"
     "<token kind=\"colon\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_bitwise,
     "~ & | ^ << >>",
     "<tokens>"
     "<token kind=\"bitwise not\" type=\"punctuator\" />"
     "<token kind=\"bitwise and or reference to\" type=\"punctuator\" />"
     "<token kind=\"bitwise or\" type=\"punctuator\" />"
     "<token kind=\"bitwise xor\" type=\"punctuator\" />"
     "<token kind=\"bitwise left shift\" type=\"punctuator\" />"
     "<token kind=\"bitwise right shift\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_assignements,
     "= += -= *= /= %= &= |= ^= <<= >>=",
     "<tokens>"
     "<token kind=\"assign\" type=\"punctuator\" />"
     "<token kind=\"add and assign\" type=\"punctuator\" />"
     "<token kind=\"subtract and assign\" type=\"punctuator\" />"
     "<token kind=\"multiply and assign\" type=\"punctuator\" />"
     "<token kind=\"divide and assign\" type=\"punctuator\" />"
     "<token kind=\"modulus and assign\" type=\"punctuator\" />"
     "<token kind=\"bitwise and and assign\" type=\"punctuator\" />"
     "<token kind=\"bitwise or and assign\" type=\"punctuator\" />"
     "<token kind=\"bitwise xor and assign\" type=\"punctuator\" />"
     "<token kind=\"bitwise left shift and assign\" type=\"punctuator\" />"
     "<token kind=\"bitwise right shift and assign\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_augment,
     "++ --",
     "<tokens>"
     "<token kind=\"augment\" type=\"punctuator\" />"
     "<token kind=\"decrement\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_bracket,
     "( ) [ ] { }",
     "<tokens>"
     "<token kind=\"open braket\" type=\"punctuator\" />"
     "<token kind=\"close braket\" type=\"punctuator\" />"
     "<token kind=\"open square braket\" type=\"punctuator\" />"
     "<token kind=\"close square braket\" type=\"punctuator\" />"
     "<token kind=\"open curly braket\" type=\"punctuator\" />"
     "<token kind=\"close curly braket\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_pointers,
     "* & . ->",
     "<tokens>"
     "<token kind=\"multiply or dereference\" type=\"punctuator\" />"
     "<token kind=\"bitwise and or reference to\" type=\"punctuator\" />"
     "<token kind=\"member access\" type=\"punctuator\" />"
     "<token kind=\"indirect member access\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_separators,
     ", ;",
     "<tokens>"
     "<token kind=\"comma\" type=\"punctuator\" />"
     "<token kind=\"semicolon\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_preprocessor,
     "nadirect # ##",
     "<tokens>"
     "<token name=\"nadirect\" type=\"identifier\" />" // stop the directive special rules
     "<token kind=\"stringize\" type=\"punctuator\" />"
     "<token kind=\"token pasting\" type=\"punctuator\" />"
     "</tokens>")

// this is parsed as the invalid a ++ ++ + b, instead of the valid a ++ + ++ b
TEST(punctuators_misleading,
     "a+++++b",
     "<tokens>"
     "<token name=\"a\" type=\"identifier\" />"
     "<token kind=\"augment\" type=\"punctuator\" />"
     "<token kind=\"augment\" type=\"punctuator\" />"
     "<token kind=\"add\" type=\"punctuator\" />"
     "<token name=\"b\" type=\"identifier\" />"
     "</tokens>")

// -- directives

TEST(directive,
     "before\n#directive\nafter",
     "<tokens>"
     "<token name=\"before\" type=\"identifier\" />"
     "<token type=\"directive start\" />"
     "<token name=\"directive\" type=\"identifier\" />"
     "<token type=\"directive end\" />"
     "<token name=\"after\" type=\"identifier\" />"
     "</tokens>")
TEST(directive_params,
     "before\n#directive a 42 b\nafter",
     "<tokens>"
     "<token name=\"before\" type=\"identifier\" />"
     "<token type=\"directive start\" />"
     "<token name=\"directive\" type=\"identifier\" />"
     "<token name=\"a\" type=\"identifier\" />"
     "<token name=\"42\" type=\"preprocessor number\" />"
     "<token name=\"b\" type=\"identifier\" />"
     "<token type=\"directive end\" />"
     "<token name=\"after\" type=\"identifier\" />"
     "</tokens>")
TEST(line_control,
     "#line 42 \"filename\"",
     "<tokens>"
     "<token type=\"directive start\" />"
     "<token name=\"line\" type=\"identifier\" />"
     "<token name=\"42\" type=\"preprocessor number\" />"
     "<token content=\"filename\" type=\"string literal\" />"
     "<token type=\"directive end\" />"
     "</tokens>")
TEST(define,
     "#define MACRO(x) strcmp(\"String Const\",x)==0",
     "<tokens>"
     "<token type=\"directive start\" />"
     "<token name=\"define\" type=\"identifier\" />"
     "<token name=\"MACRO\" type=\"identifier\" />"
     "<token kind=\"open braket\" type=\"punctuator\" />"
     "<token name=\"x\" type=\"identifier\" />"
     "<token kind=\"close braket\" type=\"punctuator\" />"
     "<token name=\"strcmp\" type=\"identifier\" />"
     "<token kind=\"open braket\" type=\"punctuator\" />"
     "<token content=\"String Const\" type=\"string literal\" />"
     "<token kind=\"comma\" type=\"punctuator\" />"
     "<token name=\"x\" type=\"identifier\" />"
     "<token kind=\"close braket\" type=\"punctuator\" />"
     "<token kind=\"equal\" type=\"punctuator\" />"
     "<token name=\"0\" type=\"preprocessor number\" />"
     "<token type=\"directive end\" />"
     "</tokens>")
TEST(header_quoted,
     "#include \"dirname\\filename\"",
     "<tokens>"
     "<token type=\"directive start\" />"
     "<token name=\"include\" type=\"identifier\" />"
     "<token angled=\"no\" name=\"dirname\\filename\" type=\"header name\" />"
     "<token type=\"directive end\" />"
     "</tokens>")
TEST(header_angled,
     "#include <dirname\\filename>",
     "<tokens>"
     "<token type=\"directive start\" />"
     "<token name=\"include\" type=\"identifier\" />"
     "<token angled=\"yes\" name=\"dirname\\filename\" type=\"header name\" />"
     "<token type=\"directive end\" />"
     "</tokens>")

// -- comments

TEST(inline_comment,
     "// this is a comment",
     "<tokens>"
     "</tokens>")
TEST(icomm_stop,
     "// this is a comment\nthis is not",
     "<tokens>"
     "<token name=\"this\" type=\"identifier\" />"
     "<token name=\"is\" type=\"identifier\" />"
     "<token name=\"not\" type=\"identifier\" />"
     "</tokens>")
TEST(icomm_identifier,
     "hello// this is a comment",
     "<tokens>"
     "<token name=\"hello\" type=\"identifier\" />"
     "</tokens>")
TEST(icomm_number,
     "53// this is a comment",
     "<tokens>"
     "<token name=\"53\" type=\"preprocessor number\" />"
     "</tokens>")
TEST(icomm_continue,
     "//this comment \\\n do not stop here \n but here",
     "<tokens>"
     "<token name=\"but\" type=\"identifier\" />"
     "<token name=\"here\" type=\"identifier\" />"
     "</tokens>")

// -- multiline comments

TEST(multiline_as_ws,
     "/* This is whitespace.... \n Even on multiple lines \r\n*/",
     "<tokens>"
     "</tokens>")
TEST(multiline_tok_divide,
     "foo/* This will divide the tokens */bar",
     "<tokens>"
     "<token name=\"foo\" type=\"identifier\" />"
     "<token name=\"bar\" type=\"identifier\" />"
     "</tokens>")
TEST(multiline_unended,
     "foo /* bar ops this is not ended...",
     "<tokens>"
     "<token name=\"foo\" type=\"identifier\" />"
     "<token msg=\"Unexpected EOF while scanning multiline comment\" severity=\"error\" type=\"error\" />"
     "</tokens>")

// -- comments and string literals

TEST(mlcomm_in_strlit,
     "\"this is /* not a comment */ \"",
     "<tokens>"
     "<token content=\"this is /* not a comment */ \" type=\"string literal\" />"
     "</tokens>")
TEST(strlit_in_mlcomm,
     "/*this is \" not a string \" */",
     "<tokens>"
     "</tokens>")
TEST(comm_in_strlit,
     "\"this is // not a comment\"",
     "<tokens>"
     "<token content=\"this is // not a comment\" type=\"string literal\" />"
     "</tokens>")
TEST(strlit_in_comm,
     "// this is \" not a string \"",
     "<tokens>"
     "</tokens>")

#endif

// -- stray chars

TEST(stray_at,
     "@",
     {EXPECTED_CONTENT, {PP_TOK_ERROR, .error = {.severity = LOG_ERROR, .msg = "Stray \'@\' in the input"}}})
TEST(stray_dollar,
     "$",
     {EXPECTED_CONTENT, {PP_TOK_ERROR, .error = {.severity = LOG_ERROR, .msg = "Stray \'$\' in the input"}}})
TEST(stray_backtick,
     "`",
     {EXPECTED_CONTENT, {PP_TOK_ERROR, .error = {.severity = LOG_ERROR, .msg = "Stray \'`\' in the input"}}})
TEST(stray_backslash,
     "\\",
     {EXPECTED_CONTENT, {PP_TOK_ERROR, .error = {.severity = LOG_ERROR, .msg = "Stray \'\\\\\' in the input"}}})

#pragma GCC diagnostic pop // -Wmissing-field-initializers

#undef TEST