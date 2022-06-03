/**
 * @file tokenizer.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Provide an interface to the tokenizer
 * @version 0.1
 * @date 2022-06-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <stdbool.h>

#include "char_stream.h"

#ifndef IDENTIFIER_MAX_LEN
#define IDENTIFIER_MAX_LEN 63 /** Number of significative chars of an identifier */
#endif

#ifndef PP_NUMBER_MAX_LEN
#define PP_NUMBER_MAX_LEN 63 /** Maximum lenght of a preprocessor number */
#endif

#ifndef STRING_LIT_MAX_LEN
#define STRING_LIT_MAX_LEN 4096 /** Maximum lenght of a string literal */
#endif

#define TOKEN_UNGET_MAX 1 /**  The maximum number of tokens that can be ungetten */
#define _TOKEN_ADD_MAX 1  /** The maximum number of tokens that can be produced (e.g. the newline after \)*/

/**
 * @brief Contains all data regardin a token stream
 */
struct tokenizer
{
    // private
    struct char_stream _source; // source of the data

    size_t _tokens_given;    // number of tokens extracted from this line (for include checking)
    bool _is_line_directive; // mark if this line is a directive (first token is #)
    bool _is_line_include;   // mark if this line is an include (directive == true, first token after DIRECTIVE_START is an `include` identifier)
    bool _is_line_define;    // mark if this line is a define (directive == true, first token after DIRECTIVE_START is a `define` identifier)

    struct pp_token_s _ungetten_token[TOKEN_UNGET_MAX + _TOKEN_ADD_MAX]; // if not NULL, will be returned before any other
    size_t _ungetten_token_len;
};

struct pp_token
{
    /**
     * @brief The type of the token
     */
    enum pp_tok_type
    {
        // Code tokens

        PP_TOK_IDENTIFIER, // keyword and identifiers
        PP_TOK_PP_NUMBER,  // preprocessor numbers
        PP_TOK_STRING_LIT, // string literals
        PP_TOK_CHAR_CONST, // char constants
        PP_TOK_HEADER,     // Header names
        PP_TOK_MACRO_NAME, // macro names
        PP_TOK_PUNCTUATOR, // punctuators

        // In-Band structure info

        PP_TOK_DIRECTIVE_START, // directive start
        PP_TOK_DIRECTIVE_STOP,  // end of directive

        PP_TOK_ERROR /**< tokenizer error. Will be thrown when we have
                      *    filename context */
    } type;

    struct bookmark mark; // Mark the start of the token

    union
    {
        // identifiers
        char name[IDENTIFIER_MAX_LEN + 1];

        // pp_number
        char *value;

        // string literals
        struct
        {
            char *value;
            size_t len;
        } string;

        // header names
        struct
        {
            char *name;     // the name of the header
            bool is_angled; // if the header is angled or not
        } header;

        // macro names
        struct
        {
            char name[IDENTIFIER_MAX_LEN + 1]; // the name of the macro
            bool is_function;                  // if the macro is function-like
        } macro_name;

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

            PUNC_BIT_NOT,          // bitwise not
            PUNC_BIT_AND_OR_REFTO, // bitwise and
            PUNC_BIT_OR,           // bitwise shift
            PUNC_BIT_XOR,          // bitwise xor
            PUNC_BIT_LSHIFT,       // bit left shift
            PUNC_BIT_RSHIFT,       // bit right shift

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
            // PUNC_REFTO -> see under bitwise
            PUNC_MEMBER_ACCESS,     // member access
            PUNC_IND_MEMBER_ACCESS, // indirect member access

            // -- SEPARATORS

            PUNC_COMMA,   // comma operator or argument separator
            PUNC_SEMICOL, // end of statement

            // -- PREPROCESSOR

            PUNC_STRINGIZE, // stringize
            PUNC_TOKPASTE,  // token pasting
        } punc_kind;
    };
};

#define PUNCTUATOR_LENGTH 3

/**
 * @brief Table of punctuators.
 *
 * Connect the text of a punctuator to its enum value
 * Terminated by a empty text
 */
static const struct
{
    char str[PUNCTUATOR_LENGTH + 1]; // Text of the punctuator
    enum punctuator_e punc;          // Enumeration value of the punctuator
} PUNCTUATORS_STRINGS[] = {
    // -- OPERATORS

    // aritmetic

    {"+", PUNC_ADD},          // add
    {"-", PUNC_SUB},          // subtract
    {"*", PUNC_MUL_OR_DEREF}, // multiply or pointer dereferencing
    {"/", PUNC_DIV},          // divide
    {"%", PUNC_MOD},          // modulus

    // comparators

    {"==", PUNC_EQ},      // equal to
    {"!=", PUNC_NEQ},     // not equal to
    {"<", PUNC_LESS},     // less than
    {"<=", PUNC_LESSEQ},  // less or equal than
    {">", PUNC_GREAT},    // greater than
    {">=", PUNC_GREATEQ}, // greater or equal than

    // logical

    {"!", PUNC_NOT},      // logical not
    {"&&", PUNC_AND},     // logical and
    {"||", PUNC_OR},      // logical or
    {"?", PUNC_QUESTION}, // question mark (for ternary logical)
    {":", PUNC_COLON},    // colon (for ternary logic and bit fields)

    // bitwise

    {"~", PUNC_BIT_NOT},          // bitwise not
    {"&", PUNC_BIT_AND_OR_REFTO}, // bitwise and or reference to
    {"|", PUNC_BIT_OR},           // bitwise shift
    {"^", PUNC_BIT_XOR},          // bitwise xor
    {"<<", PUNC_BIT_LSHIFT},      // bit left shift
    {">>", PUNC_BIT_RSHIFT},      // bit right shift

    // assignements

    {"=", PUNC_ASSIGN},              // assign to
    {"+=", PUNC_ADD_ASSIGN},         // sub and assign to
    {"-=", PUNC_SUB_ASSIGN},         // subtract and assign to
    {"*=", PUNC_MUL_ASSIGN},         // multiply and assign to
    {"/=", PUNC_DIV_ASSIGN},         // divide and assign to
    {"%=", PUNC_MOD_ASSIGN},         // modulus and assign to
    {"&=", PUNC_BIT_AND_ASSIGN},     // bitwise and and assign to
    {"|=", PUNC_BIT_OR_ASSIGN},      // bitwise or and assign to
    {"^=", PUNC_BIT_XOR_ASSIGN},     // bitwise xor and assign to
    {"<<=", PUNC_BIT_LSHIFT_ASSIGN}, // bitwise left shift and assign to
    {">>=", PUNC_BIT_RSHIFT_ASSIGN}, // bitwise right shift and assign to

    // augment and decrease

    {"++", PUNC_AUGMENT}, // augment
    {"--", PUNC_DECR},    // decrease

    // -- PARENTHESES

    {"(", PUNC_PAR_LEFT},     // left parenthese
    {")", PUNC_PAR_RIGHT},    // right parenthese
    {"[", PUNC_SQRPAR_LEFT},  // left square parenthese
    {"]", PUNC_SQRPAR_RIGHT}, // rigth square parenthese
    {"{", PUNC_CURPAR_LEFT},  // left curly parenthese
    {"}", PUNC_CURPAR_RIGHT}, // rigth curly parenthese

    // -- POINTERS AND STRUCTS

    // PUNC_DEREF -> see under aritmetic
    // PUNC_REFTO -> see under bitwise
    {".", PUNC_MEMBER_ACCESS},      // member access
    {"->", PUNC_IND_MEMBER_ACCESS}, // indirect member access

    // -- SEPARATORS

    {",", PUNC_COMMA},   // comma operator or argument separator
    {";", PUNC_SEMICOL}, // end of statement

    // -- PREPROCESSOR

    {"#", PUNC_STRINGIZE}, // stringize
    {"##", PUNC_TOKPASTE}, // token pasting

    {"", 0}}; // Terminator

#endif // _TOKENIZER_H