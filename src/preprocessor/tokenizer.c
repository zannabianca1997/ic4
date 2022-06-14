/**
 * @file tokenizer.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Break up a char stream into tokens.
 * @version 0.1
 * @date 2022-06-03
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <stdlib.h>

#include "tokenizer.h"

void tok_open(struct tokenizer *tok, struct source_stream source_name);

void tok_get(struct tokenizer *tok, struct pp_token *a);

void tok_unget(struct tokenizer *tok, struct pp_token *a);

void tok_free(struct pp_token *a)
{
    switch (a->type)
    {
    case PP_TOK_IDENTIFIER:
        free(a->name);
        break;
    case PP_TOK_PP_NUMBER:
        free(a->value);
        break;
    case PP_TOK_HEADER:
        free(a->header.name);
        break;
    case PP_TOK_MACRO_NAME:
        free(a->macro_name.name);
        break;
    case PP_TOK_STRING_LIT:
        free(a->string.value);
        break;
    }
}