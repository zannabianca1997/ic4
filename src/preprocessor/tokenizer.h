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

        PP_TOK_IDENTIFIER, // keyword and identifiers
        PP_TOK_PP_NUMBER,  // preprocessor numbers
        PP_TOK_STRING_LIT, // string literals
        PP_TOK_CHAR_CONST, // char constants
        PP_TOK_HEADER,     // Header names
        PP_TOK_PUNCTUATOR, // punctuators
        PP_TOK_OTHER,      // not recognized chars

        // Structural tokens

        PP_TOK_WHITESPACE,
        PP_TOK_NEWLINE
    } type;

    struct bookmark_s mark;

    union
    {
        // identifiers, pp_number, string literals, other
        char *content;

        // header names
        struct
        {
            char *header_name; // the name of the header
            bool is_angled;    // if the header is angled or not
        };

        // char consts
        char value;

        // Punctuators
        enum
        {
            // -- OPERATORS

            // aritmetic

            PUNC_ADD,          // add
            PUNC_SUB,          // subtract
            PUNC_MUL_OR_DEREF, // multiply or pointer dereferencing
            PUNC_DIV,          // divide
            PUNC_MOD,          // modulus

            // comparators
            PUNC_EQ,      // equal to
            PUNC_NEQ,     // not equal to
            PUNC_LESS,    // less than
            PUNC_LESSEQ,  // less or equal than
            PUNC_GREAT,   // greater than
            PUNC_GREATEQ, // greater or equal than

            // logical

            PUNC_NOT,      // logical not
            PUNC_AND,      // logical and
            PUNC_OR,       // logical or
            PUNC_QUESTION, // question mark (for ternary logical)
            PUNC_COLON,    // colon (for ternary logic and bit fields)

            // bitwise

            PUNC_BIT_NOT,    // bitwise not
            PUNC_BIT_AND,    // bitwise and
            PUNC_BIT_OR,     // bitwise shift
            PUNC_BIT_XOR,    // bitwise xor
            PUNC_BIT_LSHIFT, // bit left shift
            PUNC_BIT_RSHIFT, // bit right shift

            // assignements

            PUNC_ASSIGN,     // assign to
            PUNC_ADD_ASSIGN, // sub and assign to
            PUNC_SUB_ASSIGN, // subtract and assign to
            PUNC_MUL_ASSIGN, // multiply and assign to
            PUNC_DIV_ASSIGN, // divide and assign to
            PUNC_MOD_ASSIGN, // modulus and assign to

            // augment and decrease
            PUNC_AUGMENT, // augment
            PUNC_DECR,    // decrease

            // -- PARENTHESES

            PUNC_PAR_LEFT,     // left parenthese
            PUNC_PAR_RIGHT,    // right parenthese
            PUNC_SQRPAR_LEFT,  // left square parenthese
            PUNC_SQRPAR_RIGHT, // rigth square parenthese
            PUNC_CURPAR_LEFT,  // left curly parenthese
            PUNC_CURPAR_RIGHT, // rigth curly parenthese

            // -- POINTERS AND STRUCTS

            // PUNC_DEREF -> see under aritmetic
            PUNC_REFTO,             // reference to
            PUNC_MEMBER_ACCESS,     // member access
            PUNC_IND_MEMBER_ACCESS, // indirect member access

            // -- SEPARATORS

            PUNC_COMMA,   // comma operator or argument separator
            PUNC_SEMICOL, // end of statement

            // -- PREPROCESSOR

            PUNC_HASHTAG, // for directive start and stringize
            PUNC_TOKPASTE, // token pasting
        } kind;
    };
};

#endif