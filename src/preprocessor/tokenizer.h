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
#include "../misc/log/loglevel.h"

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

        // In-Band structure info

        PP_TOK_DIRECTIVE_START, // directive start
        PP_TOK_DIRECTIVE_STOP,  // end of directive

        PP_TOK_ERROR /**< tokenizer error. Will be thrown when we have
                      *   line and filename context */
    } type;

    struct bookmark_s mark; // Mark the start of the token

    union
    {
        // identifiers, pp_number, string literals, other
        char *content;

        // header names
        struct
        {
            char *name; // the name of the header
            bool is_angled;    // if the header is angled or not
        } header;

        // char consts
        char char_value;

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
        } punc_kind;

        // error message
        struct {
            char const *msg;
            enum loglevel_e severity; } error;
    };
};

/**
 * @brief Deallocate memory bound to token
 * Deallocate memory bound to token (e.g. string content or identifiers names)
 * @param token the token to deallocate
 */
void pp_tok_free(struct pp_token_s *token);

/**
 * @brief Compare two tokens.
 * 
 * Do not check token.mark: comparation is position indipendent
 * 
 * @param a one of the token to compare
 * @param b the other token to compare
 * @return true the two token are equal
 * @return false the two token are differen
 */
bool pp_tok_cmp(struct pp_token_s *a, struct pp_token_s *b);

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

#endif // _TOKENIZER_H