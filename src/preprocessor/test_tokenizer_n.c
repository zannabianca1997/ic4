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
__attribute__((const))
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
__attribute__((const))
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
            strcpy(msg_cur, MSG_2);
            msg_cur += strlen(msg_cur);
            msg_cur = escaped_string(msg_cur, obtained->string.value, obtained->string.len);
            strcpy(msg_cur, MSG_3);

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
__attribute__((const))
#endif
static inline const char *
check_type(struct pp_token_s *obtained, struct pp_token_s *expected){
    if(!bookmark_cmp(obtained->mark, expected->mark, CMP_EXACT, CMP_EXACT))
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
        pp_tok_free(tok), tok = pp_tokstream_get(lcontext, pp_tokstm), tokens++)
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
        const char *tok_str = pp_tok_tostring(tokens);
        char *msg = malloc(snprintf(NULL, 0, msg_fmt, tok_str));
        if (msg == NULL)
            return "Malloc failed in allocating error message";
        sprintf(msg, msg_fmt, tok_str);
        return msg;
    }

    return NULL;
}

#define TEST(TESTCASE, TEXT, EXP_TOKENS)                                       \
    struct expected_pp_token_s expected_tokens_##TESTCASE[] = EXP_TOKENS;      \
    static const char *test_tokenize_##TESTCASE()                              \
    {                                                                          \
        return _test_tokenize(#TESTCASE, TEXT, &(expected_tokens_##TESTCASE)); \
    }

TEST(space,
     " ",
     {{EXPECTED_END}})
TEST(tab,
     "\t",
     {{EXPECTED_END}})
TEST(empty_line,
     "\t   \t \t     \t",
     {{EXPECTED_END}})
TEST(newline,
     "\n",
     {{EXPECTED_END}})
TEST(empty_lines,
     "\t   \t \n\t     \t",
     {{EXPECTED_END}})
TEST(mixed_ws,
     "\n\t   \n  \r   \r\n\t \t \v",
     {{EXPECTED_END}})

#undef TEST