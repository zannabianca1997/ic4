/**
 * @file enum_strings.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-03-04
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _ENUM_STRINGS_H
#define _ENUM_STRINGS_H

#include "tokenizer.h"
#include "directives.h"

#define PP_TYPE_NAME_MAX_LEN 25
static const struct
{
    enum pp_tok_type_e type;
    const char name[PP_TYPE_NAME_MAX_LEN + 1];
} PP_TYPE_NAMES[] = {{PP_TOK_IDENTIFIER, "identifier"},
                     {PP_TOK_PP_NUMBER, "preprocessor number"},
                     {PP_TOK_STRING_LIT, "string literal"},
                     {PP_TOK_CHAR_CONST, "char constant"},
                     {PP_TOK_PUNCTUATOR, "punctuator"},
                     {PP_TOK_HEADER, "header name"},
                     {PP_TOK_MACRO_NAME, "macro name"},
                     {PP_TOK_DIRECTIVE_START, "directive start"},
                     {PP_TOK_DIRECTIVE_STOP, "directive stop"},
                     {PP_TOK_ERROR, "preprocessing error"}};

#ifdef __GNUC__
__attribute_const__
#endif
    static inline const char *
    pp_type_name(enum pp_tok_type_e type)
{
    for (size_t i = 0; i < (sizeof(PP_TYPE_NAMES) / sizeof(PP_TYPE_NAMES[0])); i++)
        if (PP_TYPE_NAMES[i].type == type)
            return PP_TYPE_NAMES[i].name;

    return "<error-type>";
}

#define PP_PUNC_KIND_NAME_MAX_LEN 35
static const struct
{
    enum punctuator_e kind;
    const char name[PP_PUNC_KIND_NAME_MAX_LEN + 1];
} PP_PUNC_KIND_NAMES[] = {{PUNC_ADD, "add"},
                          {PUNC_SUB, "subtract"},
                          {PUNC_MUL_OR_DEREF, "multiply or dereference"},
                          {PUNC_DIV, "divide"},
                          {PUNC_MOD, "modulus"},

                          {PUNC_EQ, "equal"},
                          {PUNC_NEQ, "not equal"},
                          {PUNC_LESS, "less"},
                          {PUNC_LESSEQ, "less or equal"},
                          {PUNC_GREAT, "greather"},
                          {PUNC_GREATEQ, "greather or equal"},

                          {PUNC_NOT, "not"},
                          {PUNC_AND, "and"},
                          {PUNC_OR, "or"},
                          {PUNC_QUESTION, "question mark"},
                          {PUNC_COLON, "colon"},

                          {PUNC_BIT_NOT, "bitwise not"},
                          {PUNC_BIT_AND_OR_REFTO, "bitwise and or reference to"},
                          {PUNC_BIT_OR, "bitwise or"},
                          {PUNC_BIT_XOR, "bitwise xor"},
                          {PUNC_BIT_LSHIFT, "bitwise left shift"},
                          {PUNC_BIT_RSHIFT, "bitwise right shift"},

                          {PUNC_ASSIGN, "assign"},
                          {PUNC_ADD_ASSIGN, "add and assign"},
                          {PUNC_SUB_ASSIGN, "subtract and assign"},
                          {PUNC_MUL_ASSIGN, "multiply and assign"},
                          {PUNC_DIV_ASSIGN, "divide and assign"},
                          {PUNC_MOD_ASSIGN, "modulus and assign"},
                          {PUNC_BIT_AND_ASSIGN, "bitwise and and assign"},
                          {PUNC_BIT_OR_ASSIGN, "bitwise or and assign"},
                          {PUNC_BIT_XOR_ASSIGN, "bitwise xor and assign"},
                          {PUNC_BIT_LSHIFT_ASSIGN, "bitwise left shift and assign"},
                          {PUNC_BIT_RSHIFT_ASSIGN, "bitwise right shift and assign"},

                          {PUNC_AUGMENT, "augment"},
                          {PUNC_DECR, "decrement"},

                          {PUNC_PAR_LEFT, "open braket"},
                          {PUNC_PAR_RIGHT, "close braket"},
                          {PUNC_SQRPAR_LEFT, "open square braket"},
                          {PUNC_SQRPAR_RIGHT, "close square braket"},
                          {PUNC_CURPAR_LEFT, "open curly braket"},
                          {PUNC_CURPAR_RIGHT, "close curly braket"},

                          {PUNC_MEMBER_ACCESS, "member access"},
                          {PUNC_IND_MEMBER_ACCESS, "indirect member access"},

                          {PUNC_COMMA, "comma"},
                          {PUNC_SEMICOL, "semicolon"},

                          {PUNC_STRINGIZE, "stringize"},
                          {PUNC_TOKPASTE, "token pasting"}};

#ifdef __GNUC__
__attribute_const__
#endif
    static inline const char *
    pp_punc_kind_name(enum punctuator_e kind)
{
    for (size_t i = 0; i < (sizeof(PP_PUNC_KIND_NAMES) / sizeof(PP_PUNC_KIND_NAMES[0])); i++)
        if (PP_PUNC_KIND_NAMES[i].kind == kind)
            return PP_PUNC_KIND_NAMES[i].name;

    return "<error-kind>";
}

#define DIRECTIVE_NAME_MAX_LEN 15
static const struct
{
    enum pp_directive_type_e type;
    const char name[DIRECTIVE_NAME_MAX_LEN + 1];
} DIRECTIVE_TYPE_NAMES[] = {{PP_DIRECTIVE_LINE_CTRL, "#line"},
                            {PP_DIRECTIVE_INCLUDE, "#include"},
                            {PP_DIRECTIVE_DEFINE, "#define"},
                            {PP_DIRECTIVE_UNDEF, "#undef"},
                            {PP_DIRECTIVE_IF, "#if"},
                            {PP_DIRECTIVE_ELIF, "#elif"},
                            {PP_DIRECTIVE_ELSE, "#else"},
                            {PP_DIRECTIVE_ENDIF, "#endif"},
                            {PP_DIRECTIVE_IFDEF, "#ifdef o #ifndef"},
                            {PP_DIRECTIVE_ERROR, "#error"},
                            {PP_DIRECTIVE_PRAGMA, "#pragma"},
                            {PP_DIRECTIVE_EMIT, "(internal) emit"}};

#ifdef __GNUC__
__attribute_const__
#endif
    static inline const char *
    directive_name(enum pp_directive_type_e type)
{
    for (size_t i = 0; i < (sizeof(DIRECTIVE_TYPE_NAMES) / sizeof(DIRECTIVE_TYPE_NAMES[0])); i++)
        if (DIRECTIVE_TYPE_NAMES[i].type == type)
            return DIRECTIVE_TYPE_NAMES[i].name;

    return "<error-type>";
}

#endif // _ENUM_STRINGS_H