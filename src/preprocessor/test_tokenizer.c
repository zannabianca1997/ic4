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
            xml_tag_attribute_set(tok_tag, "name", tok->name);
            break;
        case PP_TOK_PP_NUMBER:
            xml_tag_attribute_set(tok_tag, "type", "preprocessor number");
            xml_tag_attribute_set(tok_tag, "name", tok->name);
            break;
        case PP_TOK_STRING_LIT:
            xml_tag_attribute_set(tok_tag, "type", "string literal");
            xml_tag_attribute_set_with_len(tok_tag, "content", tok->string.value, tok->string.len);
            break;
        case PP_TOK_CHAR_CONST:
            xml_tag_attribute_set(tok_tag, "type", "char constant");
            xml_tag_attribute_set_with_len(tok_tag, "value", &tok->char_value, 1);
            break;
        case PP_TOK_HEADER:
            xml_tag_attribute_set(tok_tag, "type", "header name");
            xml_tag_attribute_set(tok_tag, "name", tok->header.name);
            xml_tag_attribute_set(tok_tag, "angled", tok->header.is_angled ? "yes" : "no");
            break;
        case PP_TOK_PUNCTUATOR:
            xml_tag_attribute_set(tok_tag, "type", "punctuator");
            xml_tag_attribute_set(tok_tag, "kind", "unknown");
            // finding human readable description
            for (size_t i = 0; PUNC_KIND_NAMES[i].name != NULL; i++)
                if (PUNC_KIND_NAMES[i].kind == tok->punc_kind)
                {
                    xml_tag_attribute_set(tok_tag, "kind", PUNC_KIND_NAMES[i].name);
                    break;
                }
            break;

        case PP_TOK_DIRECTIVE_START:
            xml_tag_attribute_set(tok_tag, "type", "directive start");
            break;
        case PP_TOK_DIRECTIVE_STOP:
            xml_tag_attribute_set(tok_tag, "type", "directive end");
            break;

        case PP_TOK_ERROR:
            xml_tag_attribute_set(tok_tag, "type", "error");
            xml_tag_attribute_set(tok_tag, "msg", tok->error.msg);
            switch (tok->error.severity)
            {
            case LOG_TRACE:
                xml_tag_attribute_set(tok_tag, "severity", "trace");
                break;
            case LOG_DEBUG:
                xml_tag_attribute_set(tok_tag, "severity", "debug");
                break;
            case LOG_PEDANTIC:
                xml_tag_attribute_set(tok_tag, "severity", "pedantic");
                break;
            case LOG_WARNING:
                xml_tag_attribute_set(tok_tag, "severity", "warning");
                break;
            case LOG_ERROR:
                xml_tag_attribute_set(tok_tag, "severity", "error");
                break;
            }
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
TEST(tab,
     "\t",
     "<tokens>"
     "</tokens>")
TEST(empty_line,
     "\t   \t \t     \t",
     "<tokens>"
     "</tokens>")
TEST(newline,
     "\n",
     "<tokens>"
     "</tokens>")
TEST(empty_lines,
     "\t   \t \n\t     \t",
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
     "<token name=\"hello\" type=\"identifier\" />"
     "</tokens>")
TEST(num_id,
     "by42",
     "<tokens>"
     "<token name=\"by42\" type=\"identifier\" />"
     "</tokens>")
TEST(underscore_id,
     "_contain_under___scores",
     "<tokens>"
     "<token name=\"_contain_under___scores\" type=\"identifier\" />"
     "</tokens>")
TEST(identifiers,
     "  foo\nbar32 \\\n baz_hlle ans \t _bara",
     "<tokens>"
     "<token name=\"foo\" type=\"identifier\" />"
     "<token name=\"bar32\" type=\"identifier\" />"
     "<token name=\"baz_hlle\" type=\"identifier\" />"
     "<token name=\"ans\" type=\"identifier\" />"
     "<token name=\"_bara\" type=\"identifier\" />"
     "</tokens>")

// -- preprocessor numbers

TEST(ints,
     "1 23 \n 42 456 22 \n789203\n 3",
     "<tokens>"
     "<token name=\"1\" type=\"preprocessor number\" />"
     "<token name=\"23\" type=\"preprocessor number\" />"
     "<token name=\"42\" type=\"preprocessor number\" />"
     "<token name=\"456\" type=\"preprocessor number\" />"
     "<token name=\"22\" type=\"preprocessor number\" />"
     "<token name=\"789203\" type=\"preprocessor number\" />"
     "<token name=\"3\" type=\"preprocessor number\" />"
     "</tokens>")
TEST(floats,
     "1. 2.3 \n 42. 4.56 2.2 \n78.9203\n 3.",
     "<tokens>"
     "<token name=\"1.\" type=\"preprocessor number\" />"
     "<token name=\"2.3\" type=\"preprocessor number\" />"
     "<token name=\"42.\" type=\"preprocessor number\" />"
     "<token name=\"4.56\" type=\"preprocessor number\" />"
     "<token name=\"2.2\" type=\"preprocessor number\" />"
     "<token name=\"78.9203\" type=\"preprocessor number\" />"
     "<token name=\"3.\" type=\"preprocessor number\" />"
     "</tokens>")
TEST(leading_dots,
     ".1 .23 \n .42 .456 .22 \n.789203\n .3",
     "<tokens>"
     "<token name=\".1\" type=\"preprocessor number\" />"
     "<token name=\".23\" type=\"preprocessor number\" />"
     "<token name=\".42\" type=\"preprocessor number\" />"
     "<token name=\".456\" type=\"preprocessor number\" />"
     "<token name=\".22\" type=\"preprocessor number\" />"
     "<token name=\".789203\" type=\"preprocessor number\" />"
     "<token name=\".3\" type=\"preprocessor number\" />"
     "</tokens>")
TEST(exponents,
     "1.e5 2.3e+5 \n 42e-23 .46E+34 2.2E-1 \n.789203p2\n 3.p+86",
     "<tokens>"
     "<token name=\"1.e5\" type=\"preprocessor number\" />"
     "<token name=\"2.3e+5\" type=\"preprocessor number\" />"
     "<token name=\"42e-23\" type=\"preprocessor number\" />"
     "<token name=\".46E+34\" type=\"preprocessor number\" />"
     "<token name=\"2.2E-1\" type=\"preprocessor number\" />"
     "<token name=\".789203p2\" type=\"preprocessor number\" />"
     "<token name=\"3.p+86\" type=\"preprocessor number\" />"
     "</tokens>")
TEST(strange_numbers,
     "1.e849dh34h7...34c.c34.c05 2.3...exb5 \n 0d.e.a.d.b.e.e.f .4..6E 1ex",
     "<tokens>"
     "<token name=\"1.e849dh34h7...34c.c34.c05\" type=\"preprocessor number\" />"
     "<token name=\"2.3...exb5\" type=\"preprocessor number\" />"
     "<token name=\"0d.e.a.d.b.e.e.f\" type=\"preprocessor number\" />"
     "<token name=\".4..6E\" type=\"preprocessor number\" />"
     "<token name=\"1ex\" type=\"preprocessor number\" />"
     "</tokens>")
/* 
    This exemplify how the preprocessor can mislead.
    Instead of the valid c code 0xE + 12, it is parsed as a single invalid number
*/
TEST(misleading_parse,
     "0xE+12",
     "<tokens>"
     "<token name=\"0xE+12\" type=\"preprocessor number\" />"
     "</tokens>")

// -- number and id interaction
TEST(number_seems_id,
     "123abc_def",
     "<tokens>"
     "<token name=\"123abc_def\" type=\"preprocessor number\" />"
     "</tokens>")
TEST(id_seems_number,
     "x123456",
     "<tokens>"
     "<token name=\"x123456\" type=\"identifier\" />"
     "</tokens>")

// -- string literals

TEST(strlit_so_long,
     "\"So long, and thanks for all the fish\"",
     "<tokens>"
     "<token content=\"So long, and thanks for all the fish\" type=\"string literal\" />"
     "</tokens>")
TEST(strlit_escape_chars,
     "\"\\t \\n \\r \\\" \\\' \\\\\"",
     "<tokens>"
     "<token content=\"&#x09; &#x0a; &#x0d; &quot; &apos; \\\" type=\"string literal\" />"
     "</tokens>")
TEST(strlit_escape_octals,
     "\"\\1 \\3 \\7m \\23 \\42 \\145 \\323 \\0104\"",
     "<tokens>"
     "<token content=\"&#x01; &#x03; &#x07;m &#x13; &quot; e &#xd3; &#x08;4\" type=\"string literal\" />"
     "</tokens>")
TEST(strlit_escape_hex,
     "\"\\xa \\xba \\xfam \\x30\"",
     "<tokens>"
     "<token content=\"&#x0a; &#xba; &#xfa;m 0\" type=\"string literal\" />"
     "</tokens>")
TEST(strlit_nul,
     "\" \\0 \"",
     "<tokens>"
     "<token content=\" &#x00; \" type=\"string literal\" />"
     "</tokens>")
TEST(strlit_identifier,
     "\"this is a string\"this_is_a_id",
     "<tokens>"
     "<token content=\"this is a string\" type=\"string literal\" />"
     "<token name=\"this_is_a_id\" type=\"identifier\" />"
     "</tokens>")

// -- comments

TEST(inline_comment,
     "// this is a comment",
     "<tokens>"
     "</tokens>")
TEST(icomm_stop,
     "// this is a comment\nthis is not",
     "<tokens>"
     "<token name=\"this\" type=\"identifier\" />"
     "<token name=\"is\" type=\"identifier\" />"
     "<token name=\"not\" type=\"identifier\" />"
     "</tokens>")
TEST(icomm_identifier,
     "hello// this is a comment",
     "<tokens>"
     "<token name=\"hello\" type=\"identifier\" />"
     "</tokens>")
TEST(icomm_number,
     "53// this is a comment",
     "<tokens>"
     "<token name=\"53\" type=\"preprocessor number\" />"
     "</tokens>")
TEST(icomm_continue,
     "//this comment \\\n do not stop here \n but here",
     "<tokens>"
     "<token name=\"but\" type=\"identifier\" />"
     "<token name=\"here\" type=\"identifier\" />"
     "</tokens>")

// -- multiline comments

TEST(multiline_as_ws,
     "/* This is whitespace.... \n Even on multiple lines \r\n*/",
     "<tokens>"
     "</tokens>")
TEST(multiline_tok_divide,
     "foo/* This will divide the tokens */bar",
     "<tokens>"
     "<token name=\"foo\" type=\"identifier\" />"
     "<token name=\"bar\" type=\"identifier\" />"
     "</tokens>")
TEST(multiline_unended,
     "foo /* bar ops this is not ended...",
     "<tokens>"
     "<token name=\"foo\" type=\"identifier\" />"
     "<token msg=\"Unexpected EOF while scanning multiline comment\" severity=\"error\" type=\"error\" />"
     "</tokens>")

// -- comments and string literals

TEST(mlcomm_in_strlit,
     "\"this is /* not a comment */ \"",
     "<tokens>"
     "<token content=\"this is /* not a comment */ \" type=\"string literal\" />"
     "</tokens>")
TEST(strlit_in_mlcomm,
     "/*this is \" not a string \" */",
     "<tokens>"
     "</tokens>")
TEST(comm_in_strlit,
     "\"this is // not a comment\"",
     "<tokens>"
     "<token content=\"this is // not a comment\" type=\"string literal\" />"
     "</tokens>")
TEST(strlit_in_comm,
     "// this is \" not a string \"",
     "<tokens>"
     "</tokens>")

// -- stray chars

TEST(stray_at,
     "@",
     "<tokens>"
     "<token msg=\"Stray &quot;@&quot; in the input\" severity=\"error\" type=\"error\" />"
     "</tokens>")
TEST(stray_dollar,
     "$",
     "<tokens>"
     "<token msg=\"Stray &quot;$&quot; in the input\" severity=\"error\" type=\"error\" />"
     "</tokens>")
TEST(stray_backtick,
     "`",
     "<tokens>"
     "<token msg=\"Stray &quot;`&quot; in the input\" severity=\"error\" type=\"error\" />"
     "</tokens>")
TEST(stray_backslash,
     "\\",
     "<tokens>"
     "<token msg=\"Stray &quot;\\\\&quot; in the input\" severity=\"error\" type=\"error\" />"
     "</tokens>")