/**
 * @file pp_tok_tostring.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Define a function to represent a token as a human readable string
 * @version 0.1
 * @date 2022-03-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tokenizer.h"

#include "enum_strings.h"
#include "../misc/charescape.h"

/**
 * @brief Elaborate a human readable token representation
 *
 * @param tok the token to represent
 * @return char* the representation
 */
static char *pp_tok_tostring(struct pp_token_s const *tok)
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

    case PP_TOK_MACRO_NAME:
        result = malloc(strlen(tok->macro_name.name) + (tok->macro_name.is_function) ? 2 : 1);
        if (result == NULL)
            return "Malloc failed during token string representation";
        strcpy(result, tok->macro_name.name);
        if (tok->macro_name.is_function)
            strcat(result, "(");
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
        const char *msg = (tok->type == PP_TOK_DIRECTIVE_START) ? "<directive start>" : "<directive stop>";
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
    }

    return "INTERNAL ERROR: Unknow type";
}
