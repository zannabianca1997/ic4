/**
 * @file test_tokenizer.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Test the tokenizer. 
 * @version 0.1
 * @date 2022-02-11
 * 
 * Given all the tokenizer test are basically a repeat of input a string -> tokenize -> check the result
 *  the test cases are macro generated
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lines.h"
#include "linesco.h"

#include "tokenizer.h"
#include "tokenizerco.h"

#include "../misc/xml.h"

static const struct
{
    enum punctuator_e kind;
    const char *name;
} PUNC_KIND_NAMES[] = {{PUNC_ADD, "add"},
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
                       {PUNC_BIT_AND, "bitwise and"},
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

                       {PUNC_REFTO, "reference"},
                       {PUNC_MEMBER_ACCESS, "member access"},
                       {PUNC_IND_MEMBER_ACCESS, "indirect member access"},

                       {PUNC_COMMA, "comma"},
                       {PUNC_SEMICOL, "semicolon"},

                       {PUNC_STRINGIZE, "stringize"},
                       {PUNC_TOKPASTE, "token pasting"},
                       {0, NULL}};

static const char *_test_tokenize(char const *testcase, char const *text, char const *exp_xml)
{
    context_t *lcontext = context_new(NULL, testcase);

    // opening the various streams
#pragma GCC diagnostic push
// text is a const char *, but i don't want to copy it in a buffer, and "r" guarantee it will be only read
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    FILE *text_f = fmemopen(text, strlen(text), "r");
#pragma GCC diagnostic pop
    if (text_f == NULL)
        return "Cannot open memory as stream";
    linestream_t *lines = linestream_open(lcontext, text_f);
    if (lines == NULL)
        return "Cannot open linestream";
    pp_tokstream_t *pp_tokstm = pp_tokstream_open(lcontext, lines);
    if (pp_tokstm == NULL)
        return "Cannot open tokenstream";

    // creating end buffer
    size_t out_size;
    char *out_buf;
    FILE *output = open_memstream(&out_buf, &out_size);
    if (output == NULL)
        return "open_memstream failed to open output buffer";

    // creating xml stream
    xml_tag_t *root_tag = xml_tag_new("tokens");
    if (root_tag == NULL)
        return "Cannot create root tag";
    xml_stream_t *xml_stm = xml_stream_open(output, root_tag);
    if (xml_stm == NULL)
        return "Cannot open xml stream";
    xml_tag_free(root_tag);

    // travasing streams
    for (
        struct pp_token_s *tok = pp_tokstream_get(lcontext, pp_tokstm);
        tok != NULL;
        pp_tok_free(tok), tok = pp_tokstream_get(lcontext, pp_tokstm))
    {
        // tag for tokens
        xml_tag_t *tok_tag = xml_tag_new("token");
        if (root_tag == NULL)
            return "Cannot create token tag";

        // fill with attributes
        switch (tok->type)
        {
        case PP_TOK_IDENTIFIER:
            xml_tag_attribute_set(tok_tag, "type", "identifier");
            xml_tag_attribute_set(tok_tag, "content", tok->content);
            break;
        case PP_TOK_PP_NUMBER:
            xml_tag_attribute_set(tok_tag, "type", "preprocessor number");
            xml_tag_attribute_set(tok_tag, "content", tok->content);
            break;
        case PP_TOK_STRING_LIT:
            xml_tag_attribute_set(tok_tag, "type", "string literal");
            xml_tag_attribute_set(tok_tag, "content", tok->content);
            break;
        case PP_TOK_CHAR_CONST:
            xml_tag_attribute_set(tok_tag, "type", "char constant");
            char valstr[2] = {tok->value, '\0'}; // local variabile is OK, it will be copied istantly
            xml_tag_attribute_set(tok_tag, "value", valstr);
            break;
        case PP_TOK_HEADER:
            xml_tag_attribute_set(tok_tag, "type", "header name");
            xml_tag_attribute_set(tok_tag, "name", tok->header_name);
            xml_tag_attribute_set(tok_tag, "angled", tok->is_angled ? "yes" : "no");
            break;
        case PP_TOK_PUNCTUATOR:
            xml_tag_attribute_set(tok_tag, "type", "punctuator");
            xml_tag_attribute_set(tok_tag, "kind", "unknown");
            // finding human readable description
            for (size_t i = 0; PUNC_KIND_NAMES[i].name != NULL; i++)
                if (PUNC_KIND_NAMES[i].kind == tok->kind)
                {
                    xml_tag_attribute_set(tok_tag, "kind", PUNC_KIND_NAMES[i].name);
                    break;
                }
            break;
        case PP_TOK_OTHER:
            xml_tag_attribute_set(tok_tag, "type", "other");
            xml_tag_attribute_set(tok_tag, "content", tok->content);
            break;

        case PP_TOK_DIRECTIVE_START:
            xml_tag_attribute_set(tok_tag, "type", "directive start");
            break;
        case PP_TOK_DIRECTIVE_STOP:
            xml_tag_attribute_set(tok_tag, "type", "directive end");
            break;

        case PP_TOK_ERROR:
            xml_tag_attribute_set(tok_tag, "type", "error");
            xml_tag_attribute_set(tok_tag, "msg", tok->error_msg);
            break;
        }

        // write on stream
        if (!xml_stream_tag_empty(xml_stm, tok_tag))
            return "Cannot write token tag on xml stream";
        xml_tag_free(tok_tag);
    }

    //terminate streams, and close underliyng resources
    xml_stream_close(xml_stm, true);
    pp_tokstream_close(pp_tokstm, true);

    // don't need context anymore
    context_free(lcontext);

    // now the memstream was closed by xml_stream_close, so we can check the content

    // check result
    if (strcmp(exp_xml, out_buf) == 0)
    {
        // xml_out corrispond
        free(out_buf);
        return NULL;
    }

    char *out_msg;
    size_t out_msg_len;
    FILE *out_msg_f = open_memstream(&out_msg, &out_msg_len);
    if (out_msg_f == NULL)
        return "Cannot open buffer for output message";
    fprintf(out_msg_f,
            "\nTest text was:\n\t%s\nExpected result was:\n\t%s\nObtained instead\n\t%s",
            text, exp_xml, out_buf);
    fclose(out_msg_f);

    return out_msg;
}

#define TEST(TESTCASE, TEXT, EXP_XML)                                          \
    const char *test_tokenize_##TESTCASE()                                     \
    {                                                                          \
        return _test_tokenize("Testing tokenizing " #TESTCASE, TEXT, EXP_XML); \
    }

// --- TESTS ---

// -- whitespaces

TEST(space,
     " ",
     "<tokens>"
     "</tokens>")
TEST(newline,
     "\n",
     "<tokens>"
     "</tokens>")
TEST(mixed_ws,
     "\n\t   \n  \r   \r\n\t \t \v",
     "<tokens>"
     "</tokens>")

// --- identifiers

TEST(identifier,
     "hello",
     "<tokens>"
     "<token content=\"hello\" type=\"identifier\" />"
     "</tokens>")
TEST(num_id,
     "by42",
     "<tokens>"
     "<token content=\"by42\" type=\"identifier\" />"
     "</tokens>")
TEST(underscore_id,
     "_contain_under___scores",
     "<tokens>"
     "<token content=\"_contain_under___scores\" type=\"identifier\" />"
     "</tokens>")
/*
TODO: add other/pp_num to parse this
TEST(number_wrongid,
     "9_start_wt_num",
     "<tokens>"
     "<token content=\"_contain_under___scores\" type=\"identifier\" />"
     "</tokens>")
*/
TEST(identifiers,
     "  foo\nbar32 \\\n baz_hlle \t _bara",
     "<tokens>"
     "<token content=\"foo\" type=\"identifier\" />"
     "<token content=\"bar32\" type=\"identifier\" />"
     "<token content=\"baz_hlle\" type=\"identifier\" />"
     "<token content=\"_bara\" type=\"identifier\" />"
     "</tokens>")

// -- multiline comments

TEST(multiline_as_ws,
     "/* This is whitespace.... \n Even on multiple lines \r\n*/",
     "<tokens>"
     "</tokens>")
TEST(multiline_tok_divide,
     "foo/* This will divide the tokens */bar",
     "<tokens>"
     "<token content=\"foo\" type=\"identifier\" />"
     "<token content=\"bar\" type=\"identifier\" />"
     "</tokens>")
#if 0     
//TODO: multiline and strings                                               
TEST(multiline_str_literals,                                                     
     "\"this is /* not a comment */ \"",                                         
     "<tokens>"                                                                  
     "<token content=\"this is /* not a comment */ \" type=\"string literal\" />"
     "</tokens>")
#endif
TEST(multiline_unended,
     "foo /* bar ops this is not ended...",
     "<tokens>"
     "<token content=\"foo\" type=\"identifier\" />"
     "<token msg=\"Unexpected EOF while scanning multiline comment\" type=\"error\" />"
     "</tokens>")