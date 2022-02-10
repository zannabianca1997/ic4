/**
 * @file tokenizer.h
 * @author zannabianca199712 <zannabianca199712@gmail.com>
 * @brief Generate a stream of preprocessing tokens
 * @version 0.1
 * @date 2022-02-10
 * 
 * Interface to tokenizer.c main functionalities: 
 * generation of a stream of preprocessing tokens.
 * 
 * @copyright Copyright (c) 2022
 */

#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <stdbool.h>

#include "../misc/context/context.h"
#include "../misc/bookmark.h"

// --- TOKENS ---

/**
 * @brief Contain all data relative to a single preprocesso token
 * 
 */
struct pp_token_s
{
    /**
     * @brief The type of the token 
     */
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

        // Directives detections

        PP_TOK_DIRECTIVE_START, // directive start
        PP_TOK_DIRECTIVE_STOP   // end of directive
    } type;

    struct bookmark_s mark; // Mark the start of the token

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
        enum punctuator_e
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

            PUNC_ASSIGN,            // assign to
            PUNC_ADD_ASSIGN,        // sub and assign to
            PUNC_SUB_ASSIGN,        // subtract and assign to
            PUNC_MUL_ASSIGN,        // multiply and assign to
            PUNC_DIV_ASSIGN,        // divide and assign to
            PUNC_MOD_ASSIGN,        // modulus and assign to
            PUNC_BIT_AND_ASSIGN,    // bitwise and and assign to
            PUNC_BIT_OR_ASSIGN,     // bitwise or and assign to
            PUNC_BIT_XOR_ASSIGN,    // bitwise xor and assign to
            PUNC_BIT_LSHIFT_ASSIGN, // bitwise left shift and assign to
            PUNC_BIT_RSHIFT_ASSIGN, // bitwise right shift and assign to

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

            PUNC_STRINGIZE, // stringize
            PUNC_TOKPASTE,  // token pasting
        } kind;
    };
};

/**
 * @brief Deallocate memory bound to token
 * Deallocate memory bound to token (e.g. string content or identifiers names)
 * @param token the token to deallocate
 */
void pp_tok_free(struct pp_token_s const *token);

// --- TOKENSTREAM ---

/**
 * @brief Contain all data regarding a token stream
 * 
 */
typedef struct pp_tokstream_s pp_tokstream_t;

/**
 * @brief Get the next token in the stream
 * 
 * @param context the context for which the token is needed
 * @param stream the stream from which the token is taken
 * @return struct pp_token_s* the taken token, or NULL if source has exausted
 */
struct pp_token_s *pp_tokstream_get(context_t *context, pp_tokstream_t *stream);

// --- PRINTING ---

/**
 * @brief Print the token on buf, using maximum n chars
 *
 * @param buf the destination buffer
 * @param n maximum number of character to write
 * @param token the token to print
 * 
 * @return the number of character it writed, or it would have written if n was great enough
 */
int snprintf_tok(char *buf, int n, struct pp_token_s const *token);

#ifdef DEBUG
/**
 * @brief Print a human readable representation token on buf, using maximum n chars
 *
 * @param buf the destination buffer
 * @param n maximum number of character to write
 * @param token the token to print
 * 
 * @return the number of character it writed, or it would have written if n was great enough
 */
int snprintf_repr_tok(char *buf, int n, struct pp_token_s const *token);
#endif

#endif