/*
    Generate a stream of preprocessing tokens from a stream of logical lines
*/

#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <stdbool.h>

#include "../misc/bookmark.h"

// contains all data of a single preprocessor token
struct pp_token_s
{
    // the type of the token
    enum
    {
        // Code tokens

        PP_TOK_IDENTIFIER,
        PP_TOK_PP_NUMBER,
        PP_TOK_STRING_LIT,
        PP_TOK_CHAR_CONST,
        PP_TOK_PUNCTUATOR,
        PP_TOK_OTHER,

        // Header names

        PP_TOK_HEADER_QUOTED,
        PP_TOK_HEADER_BRAKETED,

        // Structural tokens

        PP_TOK_WHITESPACE,
        PP_TOK_NEWLINE,
        PP_TOK_COMMENT
    } type;

    struct bookmark_s mark;
    char *content;
};

//TODO: write tokenizer setup struct

#endif