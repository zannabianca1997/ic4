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
            char const tag_value[2] = {tok->char_value, '\0'};
            xml_tag_attribute_set_with_len(tok_tag, "value", &tag_value[0], 2);
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

#define TEST(TESTCASE, TEXT, EXP_XML)                                              \
    const char *test_tokenize_##TESTCASE()                                         \
    {                                                                              \
        return _test_tokenize("Testing tokenizing " #TESTCASE, (TEXT), (EXP_XML)); \
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
TEST(strlit_octal_end,
     "\"\\1\" \"\\01\" \"\\001\"",
     "<tokens>"
     "<token content=\"&#x01;\" type=\"string literal\" />"
     "<token content=\"&#x01;\" type=\"string literal\" />"
     "<token content=\"&#x01;\" type=\"string literal\" />"
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
TEST(strlit_newline,
     "\"this is a string\ncutted by a newline\"",
     "<tokens>"
     "<token msg=\"Unexpected newline while scanning quoted literal\" severity=\"error\" type=\"error\" />"
     "<token name=\"cutted\" type=\"identifier\" />"
     "<token name=\"by\" type=\"identifier\" />"
     "<token name=\"a\" type=\"identifier\" />"
     "<token name=\"newline\" type=\"identifier\" />"
     "<token msg=\"Unexpected newline while scanning quoted literal\" severity=\"error\" type=\"error\" />"
     "</tokens>")

// -- char consts

TEST(chcon_letters,
     "'a' 'b' 'c'",
     "<tokens>"
     "<token type=\"char constant\" value=\"a\" />"
     "<token type=\"char constant\" value=\"b\" />"
     "<token type=\"char constant\" value=\"c\" />"
     "</tokens>")
TEST(chcon_escaped,
     "'\\42' '\\xa3' '\\n'",
     "<tokens>"
     "<token type=\"char constant\" value=\"&quot;\" />"
     "<token type=\"char constant\" value=\"&#xa3;\" />"
     "<token type=\"char constant\" value=\"&#x0a;\" />"
     "</tokens>")
TEST(chcon_zero,
     "'\\0'",
     "<tokens>"
     "<token type=\"char constant\" value=\"&#x00;\" />"
     "</tokens>")
TEST(chcon_newline,
     "'\n'",
     "<tokens>"
     "<token msg=\"Unexpected newline while scanning quoted literal\" severity=\"error\" type=\"error\" />"
     "<token msg=\"Unexpected newline while scanning quoted literal\" severity=\"error\" type=\"error\" />"
     "</tokens>")

// -- Punctuators
TEST(punctuators_aritmetics,
     "+ - / * %",
     "<tokens>"
     "<token kind=\"add\" type=\"punctuator\" />"
     "<token kind=\"subtract\" type=\"punctuator\" />"
     "<token kind=\"divide\" type=\"punctuator\" />"
     "<token kind=\"multiply or dereference\" type=\"punctuator\" />"
     "<token kind=\"modulus\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_comparators,
     "== != < <= > >=",
     "<tokens>"
     "<token kind=\"equal\" type=\"punctuator\" />"
     "<token kind=\"not equal\" type=\"punctuator\" />"
     "<token kind=\"less\" type=\"punctuator\" />"
     "<token kind=\"less or equal\" type=\"punctuator\" />"
     "<token kind=\"greather\" type=\"punctuator\" />"
     "<token kind=\"greather or equal\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_logical,
     "! && || ? :",
     "<tokens>"
     "<token kind=\"not\" type=\"punctuator\" />"
     "<token kind=\"and\" type=\"punctuator\" />"
     "<token kind=\"or\" type=\"punctuator\" />"
     "<token kind=\"question mark\" type=\"punctuator\" />"
     "<token kind=\"colon\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_bitwise,
     "~ & | ^ << >>",
     "<tokens>"
     "<token kind=\"bitwise not\" type=\"punctuator\" />"
     "<token kind=\"bitwise and or reference to\" type=\"punctuator\" />"
     "<token kind=\"bitwise or\" type=\"punctuator\" />"
     "<token kind=\"bitwise xor\" type=\"punctuator\" />"
     "<token kind=\"bitwise left shift\" type=\"punctuator\" />"
     "<token kind=\"bitwise right shift\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_assignements,
     "= += -= *= /= %= &= |= ^= <<= >>=",
     "<tokens>"
     "<token kind=\"assign\" type=\"punctuator\" />"
     "<token kind=\"add and assign\" type=\"punctuator\" />"
     "<token kind=\"subtract and assign\" type=\"punctuator\" />"
     "<token kind=\"multiply and assign\" type=\"punctuator\" />"
     "<token kind=\"divide and assign\" type=\"punctuator\" />"
     "<token kind=\"modulus and assign\" type=\"punctuator\" />"
     "<token kind=\"bitwise and and assign\" type=\"punctuator\" />"
     "<token kind=\"bitwise or and assign\" type=\"punctuator\" />"
     "<token kind=\"bitwise xor and assign\" type=\"punctuator\" />"
     "<token kind=\"bitwise left shift and assign\" type=\"punctuator\" />"
     "<token kind=\"bitwise right shift and assign\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_augment,
     "++ --",
     "<tokens>"
     "<token kind=\"augment\" type=\"punctuator\" />"
     "<token kind=\"decrement\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_bracket,
     "( ) [ ] { }",
     "<tokens>"
     "<token kind=\"open braket\" type=\"punctuator\" />"
     "<token kind=\"close braket\" type=\"punctuator\" />"
     "<token kind=\"open square braket\" type=\"punctuator\" />"
     "<token kind=\"close square braket\" type=\"punctuator\" />"
     "<token kind=\"open curly braket\" type=\"punctuator\" />"
     "<token kind=\"close curly braket\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_pointers,
     "* & . ->",
     "<tokens>"
     "<token kind=\"multiply or dereference\" type=\"punctuator\" />"
     "<token kind=\"bitwise and or reference to\" type=\"punctuator\" />"
     "<token kind=\"member access\" type=\"punctuator\" />"
     "<token kind=\"indirect member access\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_separators,
     ", ;",
     "<tokens>"
     "<token kind=\"comma\" type=\"punctuator\" />"
     "<token kind=\"semicolon\" type=\"punctuator\" />"
     "</tokens>")
TEST(punctuators_preprocessor,
     "nadirect # ##",
     "<tokens>"
     "<token name=\"nadirect\" type=\"identifier\" />" // stop the directive special rules
     "<token kind=\"stringize\" type=\"punctuator\" />"
     "<token kind=\"token pasting\" type=\"punctuator\" />"
     "</tokens>")

// this is parsed as the invalid a ++ ++ + b, instead of the valid a ++ + ++ b
TEST(punctuators_misleading,
     "a+++++b",
     "<tokens>"
     "<token name=\"a\" type=\"identifier\" />"
     "<token kind=\"augment\" type=\"punctuator\" />"
     "<token kind=\"augment\" type=\"punctuator\" />"
     "<token kind=\"add\" type=\"punctuator\" />"
     "<token name=\"b\" type=\"identifier\" />"
     "</tokens>")

// -- directives

TEST(directive,
     "before\n#directive\nafter",
     "<tokens>"
     "<token name=\"before\" type=\"identifier\" />"
     "<token type=\"directive start\" />"
     "<token name=\"directive\" type=\"identifier\" />"
     "<token type=\"directive end\" />"
     "<token name=\"after\" type=\"identifier\" />"
     "</tokens>")
TEST(directive_params,
     "before\n#directive a 42 b\nafter",
     "<tokens>"
     "<token name=\"before\" type=\"identifier\" />"
     "<token type=\"directive start\" />"
     "<token name=\"directive\" type=\"identifier\" />"
     "<token name=\"a\" type=\"identifier\" />"
     "<token name=\"42\" type=\"preprocessor number\" />"
     "<token name=\"b\" type=\"identifier\" />"
     "<token type=\"directive end\" />"
     "<token name=\"after\" type=\"identifier\" />"
     "</tokens>")
TEST(line_control,
     "#line 42 \"filename\"",
     "<tokens>"
     "<token type=\"directive start\" />"
     "<token name=\"line\" type=\"identifier\" />"
     "<token name=\"42\" type=\"preprocessor number\" />"
     "<token content=\"filename\" type=\"string literal\" />"
     "<token type=\"directive end\" />"
     "</tokens>")
TEST(define,
     "#define MACRO(x) strcmp(\"String Const\",x)==0",
     "<tokens>"
     "<token type=\"directive start\" />"
     "<token name=\"define\" type=\"identifier\" />"
     "<token name=\"MACRO\" type=\"identifier\" />"
     "<token kind=\"open braket\" type=\"punctuator\" />"
     "<token name=\"x\" type=\"identifier\" />"
     "<token kind=\"close braket\" type=\"punctuator\" />"
     "<token name=\"strcmp\" type=\"identifier\" />"
     "<token kind=\"open braket\" type=\"punctuator\" />"
     "<token content=\"String Const\" type=\"string literal\" />"
     "<token kind=\"comma\" type=\"punctuator\" />"
     "<token name=\"x\" type=\"identifier\" />"
     "<token kind=\"close braket\" type=\"punctuator\" />"
     "<token kind=\"equal\" type=\"punctuator\" />"
     "<token name=\"0\" type=\"preprocessor number\" />"
     "<token type=\"directive end\" />"
     "</tokens>")
TEST(header_quoted,
     "#include \"dirname\\filename\"",
     "<tokens>"
     "<token type=\"directive start\" />"
     "<token name=\"include\" type=\"identifier\" />"
     "<token angled=\"no\" name=\"dirname\\filename\" type=\"header name\" />"
     "<token type=\"directive end\" />"
     "</tokens>")
TEST(header_angled,
     "#include <dirname\\filename>",
     "<tokens>"
     "<token type=\"directive start\" />"
     "<token name=\"include\" type=\"identifier\" />"
     "<token angled=\"yes\" name=\"dirname\\filename\" type=\"header name\" />"
     "<token type=\"directive end\" />"
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