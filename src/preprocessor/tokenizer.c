/**
 * @file tokenizer.c
 * @author zannabianca199712 <zannabianca199712@gmail.com>
 * @brief Generate a stream of preprocessing tokens
 * @version 0.1
 * @date 2022-02-10
 * 
 * Read a linestream and break it into preprocessing tokens
 * 
 * @copyright Copyright (c) 2022
 */

#include <stdlib.h>
#include <stdbool.h> // bool
#include <ctype.h>   // isalpha, isalnum, isdigit

#include "../misc/context/context.h"
#include "../misc/bookmark.h"
#include "../misc/log/log.h"

#include "messages.cat.h"

#include "tokenizer.h"
#include "tokenizerco.h"

#include "lines.h"
#include "linesco.h"

#define PUNCTUATOR_LENGTH 3

/**
 * @brief Table of punctuators
 * 
 * Connect the text of a punctuator to its enum value
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

    {"~", PUNC_BIT_NOT},     // bitwise not
    {"&", PUNC_BIT_AND},     // bitwise and
    {"|", PUNC_BIT_OR},      // bitwise shift
    {"^", PUNC_BIT_XOR},     // bitwise xor
    {"<<", PUNC_BIT_LSHIFT}, // bit left shift
    {">>", PUNC_BIT_RSHIFT}, // bit right shift

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
    {"&", PUNC_REFTO},              // reference to
    {".", PUNC_MEMBER_ACCESS},      // member access
    {"->", PUNC_IND_MEMBER_ACCESS}, // indirect member access

    // -- SEPARATORS

    {",", PUNC_COMMA},   // comma operator or argument separator
    {";", PUNC_SEMICOL}, // end of statement

    // -- PREPROCESSOR

    {"#", PUNC_STRINGIZE}, // stringize
    {"##", PUNC_TOKPASTE}, // token pasting
};

/**
 * @brief Contain the data for a token stream
 */
struct pp_tokstream_s
{
    linestream_t *source; // the source of the lines

    struct logical_line_s *current_line; // the line that's being tokenized
    size_t cursor;                       // pointing to the next char that need to be processed
    bool is_line_directive;              // mark if this line is a directive (first non-whitespace character is #)
};

// --- STREAM MANAGING ---

pp_tokstream_t *pp_tokstream_open(context_t *context, linestream_t *source)
{
    context_t *lcontext = context_new(context, TOKENIZER_CONTEXT_OPENING);

    pp_tokstream_t *new_stream = malloc(sizeof(pp_tokstream_t));
    if (new_stream == NULL)
        log_error(lcontext, TOKENIZER_MALLOC_FAIL_OPEN);

    new_stream->source = source;
    new_stream->current_line = NULL;

    context_free(lcontext);

    return new_stream;
}

void pp_tokstream_close(pp_tokstream_t *stream, bool recursive_close){
    if(recursive_close)
    linestream_close(stream->source,recursive_close);

    line_free(stream->current_line);
    free(stream);
}