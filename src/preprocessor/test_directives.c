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

#include <stddef.h>
#include <string.h>

#include "../misc/bookmark.h"
#include "../misc/log/loglevel.h"
#include "../misc/context/context.h"

#include "lines.h"
#include "linesco.h"

#include "tokenizer.h"
#include "tokenizerco.h"

#include "directives.h"

#include "enum_strings.h"
#include "../misc/charescape.h"

#define TESTS_MAX_DIRECTIVE_ARGS 10

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
                    struct pp_token_s args[];
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
                    struct pp_token_s args[];
                };
            };
        } include;

        struct
        {
            char *macro_name;
            bool is_function;

            char *args[TESTS_MAX_DIRECTIVE_ARGS];
            size_t nargs;

            size_t ntokens;
            struct pp_token_s tokens[];
        } define;

        struct
        {
            enum loglevel_e severity;
            char *msg;
        } error;

        struct
        {
            size_t nargs;
            struct pp_token_s args[];
        };
    };
};

/**
 * @brief check a directive against an expected directive
 * 
 * @param obtained the directive to check
 * @param expected the expected directive
 * @return true the directive match
 * @return false the directive did not match
 */
static bool check_directive(struct pp_directive_s const *obtained, struct pp_expected_directive_s const *expected)
{
    if (expected->compare_type == EXPECTED_IGNORE)
        return true;

    if (expected->compare_type == EXPECTED_TYPE)
        return obtained->type == expected->type;

    if (expected->compare_type == EXPECTED_MARK)
        return bookmark_cmp(obtained->mark, expected->mark, CMP_EXACT, CMP_EXACT);

    // remain EXPECTED_EXACT and EXPECTED_CONTENT

    if (expected->compare_type == EXPECTED_EXACT)
        if (!bookmark_cmp(obtained->mark, expected->mark, CMP_EXACT, CMP_EXACT))
            return false;

    // now we need to check content alone

    if (obtained->type != expected->type)
        return false;

    switch (obtained->type)
    {
    case PP_DIRECTIVE_LINE_CTRL:
        if (obtained->line_ctrl.need_macros != expected->line_ctrl.need_macros)
            return false;
        if (obtained->line_ctrl.need_macros)
        {
            if (obtained->line_ctrl.nargs != expected->line_ctrl.nargs)
                return false;
            for (size_t i = 0; i < obtained->line_ctrl.nargs; i++)
                if (!pp_tok_cmp(obtained->line_ctrl.args[i], &(expected->line_ctrl.args[i])))
                    return false;
        }
        else if (obtained->line_ctrl.line_num != expected->line_ctrl.line_num ||
                 strcmp(obtained->line_ctrl.file_name, expected->line_ctrl.file_name) != 0)
            return false;

        break;

    case PP_DIRECTIVE_INCLUDE:
        if (obtained->include.need_macros != expected->include.need_macros)
            return false;
        if (obtained->include.need_macros)
        {
            if (obtained->include.nargs != expected->include.nargs)
                return false;
            for (size_t i = 0; i < obtained->include.nargs; i++)
                if (!pp_tok_cmp(obtained->include.args[i], &(expected->include.args[i])))
                    return false;
        }
        else if (obtained->include.is_angled != obtained->include.is_angled ||
                 strcmp(obtained->include.file_name, obtained->include.file_name) != 0)
            return false;
        break;

    case PP_DIRECTIVE_DEFINE:
        if (obtained->define.ntokens != expected->define.ntokens ||
            obtained->define.is_function != expected->define.is_function ||
            strcmp(obtained->define.macro_name, expected->define.macro_name) != 0)
            return false;

        // checking definition tokens
        for (size_t i = 0; i < obtained->define.ntokens; i++)
            if (!pp_tok_cmp(obtained->define.tokens[i], &(expected->define.tokens[i])))
                return false;

        // if function-like, checking args
        if (obtained->define.is_function)
        {
            if (obtained->define.nargs != expected->define.nargs)
                return false;
            for (size_t i = 0; i < obtained->define.args; i++)
                if (strcmp(obtained->define.args[i], &(expected->define.args[i])) != 0)
                    return false;
        }
        break;

    case PP_DIRECTIVE_ERROR:
        if (obtained->error.severity != expected->error.severity)
            return false;
        if (strcmp(obtained->error.msg, expected->error.msg) != 0)
            return false;
        break;

    // these directive use only generic arguments. No difference in checking...
    case PP_DIRECTIVE_IF:
    case PP_DIRECTIVE_ELIF:
    case PP_DIRECTIVE_PRAGMA:
        if (obtained->nargs != expected->nargs)
            return false;
        for (size_t i = 0; i < obtained->args; i++)
            if (strcmp(obtained->args[i], &(expected->args[i])) != 0)
                return false;
        break;
    
    // these directive are contentless, no check is required
    case PP_DIRECTIVE_ELSE:
    case PP_DIRECTIVE_ENDIF:
    break;
    }

    return true;
}

