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

#include <stdlib.h>  // malloc, realloc, free
#include <stdbool.h> // bool
#include <ctype.h>   // isalpha, isalnum, isdigit, isspace, isprint
#include <string.h>  // strlen

#if 0
#pragma GCC warning "<stdio.h> included for debug purposes"
#include <stdio.h>
#endif

#include "../misc/context/context.h"
#include "../misc/bookmark.h"
#include "../misc/log/log.h"
#include "../misc/charescape.h"

#include "messages.cat.h"

#include "tokenizer.h"
#include "tokenizerco.h"

#include "lines.h"

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

/**
 * @brief Contain the data for a token stream
 */
struct pp_tokstream_s
{
    linestream_t *source; // the source of the lines

    struct logical_line_s *current_line; // the line that's being tokenized
    size_t cursor;                       // pointing to the next char that need to be processed

    size_t tokens_given;    // number of tokens extracted (for include checking)
    bool is_line_directive; // mark if this line is a directive (first token is #)
    bool is_line_include;   // mark if this line is an include (directive == true, first token after DIRECTIVE_START is an `include` identifier)
    bool is_line_define;    // mark if this line is a define (directive == true, first token after DIRECTIVE_START is a `define` identifier)

    struct pp_token_s *ungetten_token;        // if not NULL, will be returned before any other
    struct pp_token_s *delayed_directive_end; // if not NULL, will be returned after the ungetten token and before any other
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

    new_stream->delayed_directive_end = NULL;

    context_free(lcontext);

    return new_stream;
}

void pp_tokstream_close(pp_tokstream_t *stream, bool recursive_close)
{
    if (recursive_close)
        linestream_close(stream->source, recursive_close);
    if (stream->current_line != NULL)
        line_free(stream->current_line);
    if (stream->delayed_directive_end != NULL)
        pp_tok_free(stream->delayed_directive_end);
    free(stream);
}

// --- TOKEN END OF LIFE ---

void pp_tok_free(struct pp_token_s *token)
{
    if (token->type == PP_TOK_IDENTIFIER ||
        token->type == PP_TOK_PP_NUMBER)
        free(token->name);
    if (token->type == PP_TOK_STRING_LIT)
        free(token->string.value);
    if (token->type == PP_TOK_HEADER)
        free(token->header.name);
    if (token->type == PP_TOK_ERROR)
        free(token->error.msg);
    free(token);
}

// --- TOKEN COMPARATION ---

bool pp_tok_cmp(struct pp_token_s const *a, struct pp_token_s const *b)
{
    if (a->type != b->type)
        return false;

    switch (a->type)
    {
    case PP_TOK_IDENTIFIER:
    case PP_TOK_PP_NUMBER:
    case PP_TOK_STRING_LIT:
        return strcmp(a->name, b->name) == 0;

    case PP_TOK_CHAR_CONST:
        return a->char_value == b->char_value;

    case PP_TOK_HEADER:
        return a->header.is_angled == b->header.is_angled && strcmp(a->header.name, b->header.name) == 0;

    case PP_TOK_MACRO_NAME:
        return a->macro_name.is_function == b->macro_name.is_function && strcmp(a->macro_name.name, b->macro_name.name) == 0;

    case PP_TOK_PUNCTUATOR:
        return a->punc_kind == b->punc_kind;

    case PP_TOK_DIRECTIVE_START:
    case PP_TOK_DIRECTIVE_STOP:
        return true; // contentless

    case PP_TOK_ERROR:
        return a->error.severity == b->error.severity && strcmp(a->error.msg, b->error.msg) == 0;
    }

    log_error(NULL, TOKENIZER_CMP_UNKNOW);
}

// --- UTILITY ---

#ifdef __GNUC__
__attribute_const__
#endif
    /**
     * @brief Check if a char can be part of an identifier
     * @param ch the char to check
     */
    static bool
    is_identifier_char(char ch)
{
    return isalnum(ch) || (ch == '_');
}

#ifdef __GNUC__
__attribute_const__
#endif
    /**
     * @brief Check if a char can be the start of an identifier
     * @param ch the char to check
     */
    static bool
    is_identifier_start_char(char ch)
{
    return isalpha(ch) || (ch == '_');
}

#ifdef __GNUC__
__attribute_const__
#endif
    /**
     * @brief Check if a char is a sign
     * @param ch the char to check
     */
    static bool
    is_sign(char ch)
{
    return (ch == '+') || (ch == '-');
}

#ifdef __GNUC__
__attribute_const__
#endif
    /**
     * @brief Check if a char can be start of an exponent
     * @param ch the char to check
     */
    static bool
    is_exp_start(char ch)
{
    return (ch == 'e') || (ch == 'E') ||
           (ch == 'p') || (ch == 'P');
}

/**
 * @brief Check if a string constain a given substring at the given index
 *
 * @param string the string to check
 * @param start the start of the substring
 * @param substr the substring
 * @return true the substring is present at the given index
 * @return false the substring is not present at the given index
 */
static bool contains_at(const char *restrict string, size_t start, const char *restrict substr)
{
    string += start;
    while (*substr)
        if (*substr++ != *string++)
            return false;
    return true;
}

// --- TOKEN PARSING FUNCTIONS ---

/**
 * @brief The type of a funtion that parse a type of token.
 *
 * @param context the context in which the token is parsed
 * @param stream the stream from which the token is parsed
 * @param n the number of chars the token occupy
 * @return the token parsed, or NULL if no token is to emit. token.mark is uninizialized
 */
typedef struct pp_token_s *(*parsing_fun_ptr_t)(context_t *context, struct pp_tokstream_s const *stream, size_t *n);

// eat all whitespace, give no tokens
static struct pp_token_s *parse_whitespace(
#ifdef __GNUC__
    __attribute__((unused))
#endif
    context_t *context,
    struct pp_tokstream_s const *stream, size_t *n)
{
    size_t space_len = 0;
    while (stream->current_line->content[stream->cursor + space_len] != '\0' &&
           isspace(stream->current_line->content[stream->cursor + space_len]))
        space_len++;

    *n = space_len;
    return NULL;
}

/* TODO: both parse_identifier, parse_pp_number, parse_quoted and parse_header_name use a dinamically allocated buffer
         of the same lenght of the line. maybe use open_memstream or a ANSI growing buffer?
         Another solution would be to scan the line first to measure the content, then copy it into the rigth size buffer */

// parse an identifier
static struct pp_token_s *parse_identifier(context_t *context, struct pp_tokstream_s const *stream, size_t *n)
{
    // short circuit
    if (!is_identifier_start_char(stream->current_line->content[stream->cursor]))
    {
        *n = 0;
        return NULL;
    }

    context_t *lcontext = context_new(context, TOKENIZER_CONTEXT_IDENTIFIER);

    // allocate space for the identifier
    char *name = malloc(strlen(stream->current_line->content) - stream->cursor + 1);
    if (name == NULL)
        log_error(lcontext, TOKENIZER_MALLOC_FAIL_IDENTIFIER);

    // copy chars until they can be copied
    size_t namelen = 0;
    do
    {
        name[namelen] = stream->current_line->content[stream->cursor + namelen];
        namelen++;
    } while (stream->current_line->content[stream->cursor + namelen] != '\0' &&
             is_identifier_char(stream->current_line->content[stream->cursor + namelen]));

    // 0-terminate name
    name[namelen] = '\0';

    // shrink name
    char *new_name = realloc(name, namelen + 1);
    if (new_name == NULL)
        log_warning(lcontext, TOKENIZER_MALLOC_FAIL_SHRINKIDENTIFIER); // continue with unshrinked name
    else
        name = new_name;

    // create token
    struct pp_token_s *new_token = malloc(sizeof(struct pp_token_s));
    if (new_token == NULL)
        log_error(lcontext, TOKENIZER_MALLOC_FAIL_TOKEN);

    new_token->type = PP_TOK_IDENTIFIER;
    new_token->name = name;

    context_free(lcontext);

    *n = namelen;
    return new_token;
}

// parse a preprocessor number
static struct pp_token_s *parse_pp_number(context_t *context, struct pp_tokstream_s const *stream, size_t *n)
{
    // short circuit
    if (!(stream->current_line->content[stream->cursor] == '.' || isdigit(stream->current_line->content[stream->cursor])))
    {
        // preprocessing numbers must begin with a period or a digit
        *n = 0;
        return NULL;
    }

    context_t *lcontext = context_new(context, TOKENIZER_CONTEXT_PP_NUMBER);

    // allocate space for the name
    char *name = malloc(strlen(stream->current_line->content) - stream->cursor + 1);
    if (name == NULL)
        log_error(lcontext, TOKENIZER_MALLOC_FAIL_PP_NUMBER);

    // count copied chars
    size_t numlen = 0;

    // consume optional perion
    if (stream->current_line->content[stream->cursor + numlen] == '.')
        name[numlen++] = '.';

    // check required digit
    if (!isdigit(stream->current_line->content[stream->cursor + numlen]))
    {
        *n = 0;
        return NULL;
    }

    do
    {
        name[numlen] = stream->current_line->content[stream->cursor + numlen];
        numlen++;
    } while (stream->current_line->content[stream->cursor + numlen] != '\0' &&              // if the line is still going and
             (is_identifier_char(stream->current_line->content[stream->cursor + numlen]) || // next char is digit, or alpha, or underscore
              stream->current_line->content[stream->cursor + numlen] == '.' ||              // or a period
              (is_sign(stream->current_line->content[stream->cursor + numlen]) &&           // ora a sign, but only if
               is_exp_start(stream->current_line->content[stream->cursor + numlen - 1])))); // preceeded by a exponent start

    // 0-terminate name
    name[numlen] = '\0';

    // shrink name
    char *new_content = realloc(name, numlen + 1);
    if (new_content == NULL)
        log_warning(lcontext, TOKENIZER_MALLOC_FAIL_SHRINKPP_NUMBER); // continue with unshrinked name
    else
        name = new_content;

    // create token
    struct pp_token_s *new_token = malloc(sizeof(struct pp_token_s));
    if (new_token == NULL)
        log_error(lcontext, TOKENIZER_MALLOC_FAIL_TOKEN);

    new_token->type = PP_TOK_PP_NUMBER;
    new_token->name = name;

    context_free(lcontext);

    *n = numlen;
    return new_token;
}

/**
 * @brief Parse a quoted string or char literal
 *
 * @param quote the open quote (" or ' or <)
 * @param quote the close quote (" or ' or >)
 * @param escape if escaped sequence are parsed
 * @return struct pp_token_s* if non-null, the parsed quote
 */
static struct pp_token_s *parse_quoted(context_t *lcontext, struct pp_tokstream_s const *stream, size_t *n, char open_quote, char close_quote, bool escape)
{
    // short circuit
    if (!(stream->current_line->content[stream->cursor] == open_quote))
    {
        // string literals must begin with a "
        *n = 0;
        return NULL;
    }

    // create token
    struct pp_token_s *new_token = malloc(sizeof(struct pp_token_s));
    if (new_token == NULL)
        log_error(lcontext, TOKENIZER_MALLOC_FAIL_TOKEN);

    // allocate space for the content
    char *content = malloc(strlen(stream->current_line->content) - stream->cursor + 1);
    if (content == NULL)
        log_error(lcontext, TOKENIZER_MALLOC_FAIL_STRING);

    // count readed chars
    size_t takenchars = 1; // already took the initial "
    // count written chars
    size_t writtenchars = 0; // are different from takenchars cause escape sequences

    while (stream->current_line->content[stream->cursor + takenchars] != close_quote)
    {
        if (stream->current_line->content[stream->cursor + takenchars] == '\0')
        {
            // newline during quoted literal
            new_token->type = PP_TOK_ERROR;

            new_token->error.msg = malloc(strlen(TOKENIZER_NL_STRINGLIT) + 1);
            if (new_token->error.msg == NULL)
                log_error(lcontext, TOKENIZER_MALLOC_FAIL_ERROR);
            strcpy(new_token->error.msg, TOKENIZER_NL_STRINGLIT);

            new_token->error.severity = LOG_ERROR;

            free(content);

            *n = takenchars;
            return new_token;
        }
        else if (escape && stream->current_line->content[stream->cursor + takenchars] == '\\')
        {
            // start of escape char
            takenchars++;

            // we know it's not the end of the line cause
            // it would be an escaped newline, so we can safely check next char
            switch (stream->current_line->content[stream->cursor + takenchars++])
            {
            case 't':
                content[writtenchars++] = '\t';
                break;
            case 'n':
                content[writtenchars++] = '\n';
                break;
            case 'r':
                content[writtenchars++] = '\r';
                break;
            case 'v':
                content[writtenchars++] = '\v';
                break;
            case 'f':
                content[writtenchars++] = '\f';
                break;
            case '\"':
                content[writtenchars++] = '\"';
                break;
            case '\'':
                content[writtenchars++] = '\'';
                break;
            case '\\':
                content[writtenchars++] = '\\';
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            {
                // octal digit
                char ch = stream->current_line->content[stream->cursor + takenchars - 1] - '0';
                // get up to other two octal digits
                char nextc = stream->current_line->content[stream->cursor + takenchars];
                if (nextc >= '0' && nextc <= '7')
                {
                    takenchars++;
                    ch = 8 * ch + (nextc - '0');
                    nextc = stream->current_line->content[stream->cursor + takenchars];
                    if (nextc >= '0' && nextc <= '7')
                    {
                        takenchars++;
                        ch = 8 * ch + (nextc - '0');
                    }
                }
                // put the readed code in the string
                content[writtenchars++] = ch;
                break;
            }

            case 'x':
            {
                // hex digit
                char ch = 0, nextc;
                // implementation require us to take as many hex digits are there
                while (isxdigit(nextc = stream->current_line->content[stream->cursor + takenchars]))
                {
                    takenchars++;
                    switch (nextc)
                    {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        ch = 16 * ch + (nextc - '0');
                        break;
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'e':
                    case 'f':
                        ch = 16 * ch + (nextc + 10 - 'a');
                        break;
                    case 'A':
                    case 'B':
                    case 'C':
                    case 'D':
                    case 'E':
                    case 'F':
                        ch = 16 * ch + (nextc + 10 - 'A');
                        break;
                    }
                }
                // put the readed code in the string
                content[writtenchars++] = ch;
                break;
            }

            case 'u':
                // unicode literals are unimplemented
            default:
            {
                // unknow escape sequence
                new_token->type = PP_TOK_ERROR;

                if (stream->current_line->content[stream->cursor + takenchars - 1] == 'u')
                {
                    new_token->error.msg = malloc(strlen(TOKENIZER_ESCAPE_UNICODE) + 1);
                    if (new_token->error.msg == NULL)
                        log_error(lcontext, TOKENIZER_MALLOC_FAIL_ERROR);
                    strcpy(new_token->error.msg, TOKENIZER_ESCAPE_UNICODE);
                }
                else
                {
                    new_token->error.msg = malloc(strlen(TOKENIZER_ESCAPE_UNKNOW) + 1);
                    if (new_token->error.msg == NULL)
                        log_error(lcontext, TOKENIZER_MALLOC_FAIL_ERROR);
                    strcpy(new_token->error.msg, TOKENIZER_ESCAPE_UNKNOW);
                }

                new_token->error.severity = LOG_ERROR;
                free(content);

                *n = takenchars;
                return new_token;
            }
            }
        }
        else
            // simple copy
            content[writtenchars++] = stream->current_line->content[stream->cursor + takenchars++];
    }
    takenchars++; // take the last "

    // 0-terminate content
    content[writtenchars] = '\0';

    // shrink content
    char *new_content = realloc(content, writtenchars + 1);
    if (new_content == NULL)
        log_warning(lcontext, TOKENIZER_MALLOC_FAIL_SHRINKSTRING); // continue with unshrinked content
    else
        content = new_content;

    new_token->type = PP_TOK_STRING_LIT;
    new_token->string.value = content;
    new_token->string.len = writtenchars + 1;

    *n = takenchars;
    return new_token;
}

// parse a string literal
static struct pp_token_s *parse_string_literal(context_t *context, struct pp_tokstream_s const *stream, size_t *n)
{
    context_t *lcontext = context_new(context, TOKENIZER_CONTEXT_STRING);
    struct pp_token_s *parsed = parse_quoted(lcontext, stream, n, '\"', '\"', true);
    context_free(lcontext);
    return parsed;
}

// parse a char literal
static struct pp_token_s *parse_char_literal(context_t *context, struct pp_tokstream_s const *stream, size_t *n)
{
    context_t *lcontext = context_new(context, TOKENIZER_CONTEXT_STRING);
    struct pp_token_s *parsed = parse_quoted(lcontext, stream, n, '\'', '\'', true);

    if (parsed != NULL && parsed->type == PP_TOK_STRING_LIT)
    {
        // parsing went OK
        if (parsed->string.len != 2)
        {
            // multi char string literal
            parsed->type = PP_TOK_ERROR;
            parsed->error.severity = LOG_ERROR;
            parsed->error.msg = malloc(strlen(TOKENIZER_MULTI_CH_CHAR_LIT) + 1);
            if (parsed->error.msg == NULL)
                log_error(lcontext, TOKENIZER_MALLOC_FAIL_ERROR);
            strcpy(parsed->error.msg, TOKENIZER_MULTI_CH_CHAR_LIT);
        }
        else
        {
            // recovering content
            char ch = parsed->string.value[0];
            free(parsed->string.value);

            // changing token
            parsed->type = PP_TOK_CHAR_CONST;
            parsed->char_value = ch;
        }
    }

    context_free(lcontext);
    return parsed;
}

// parse a punctuator
static struct pp_token_s *parse_punctuator(context_t *context, struct pp_tokstream_s const *stream, size_t *restrict n)
{
    // short circuit
    if (isspace(stream->current_line->content[stream->cursor]) || isalnum(stream->current_line->content[stream->cursor]))
    {
        *n = 0;
        return NULL;
    }

    context_t *lcontext = context_new(context, TOKENIZER_CONTEXT_PUNCT);

    struct pp_token_s *new_token = NULL;
    *n = 0;

    for (size_t i = 0; strlen(PUNCTUATORS_STRINGS[i].str) > 0; i++)                                 // for all the punctuators
        if (strlen(PUNCTUATORS_STRINGS[i].str) > *n &&                                              // if this punctuator is longer
            contains_at(stream->current_line->content, stream->cursor, PUNCTUATORS_STRINGS[i].str)) // and it's present at the given point
        {
            *n = strlen(PUNCTUATORS_STRINGS[i].str);
            if (new_token == NULL)
            {
                new_token = malloc(sizeof(struct pp_token_s));
                if (new_token == NULL)
                    log_error(lcontext, TOKENIZER_MALLOC_FAIL_TOKEN);
                new_token->type = PP_TOK_PUNCTUATOR;
            }
            new_token->punc_kind = PUNCTUATORS_STRINGS[i].punc;
        }

    context_free(lcontext);
    return new_token;
}

// eat a comment, give no token
static struct pp_token_s *parse_comment(
#ifdef __GNUC__
    __attribute__((unused))
#endif
    context_t *context,
    struct pp_tokstream_s const *stream, size_t *n)
{
    if (!(stream->current_line->content[stream->cursor] == '/' &&
          stream->current_line->content[stream->cursor + 1] == '/'))
    {
        *n = 0;
        return NULL; // not a comment
    }

    // eat all the line unconditionally
    *n = strlen(stream->current_line->content) - stream->cursor;
    return NULL;
}

// parse a header name
static struct pp_token_s *parse_header_name(context_t *context, struct pp_tokstream_s const *stream, size_t *restrict n)
{

    // short circuit
    if (
        !stream->is_line_include ||
        (stream->current_line->content[stream->cursor] != '<' &&
         stream->current_line->content[stream->cursor] != '\"'))
    {
        *n = 0;
        return NULL;
    }

    bool const angled = stream->current_line->content[stream->cursor] == '<';

    context_t *lcontext = context_new(context, TOKENIZER_CONTEXT_HEADER_NAME);

    struct pp_token_s *parsed = parse_quoted(lcontext, stream, n, angled ? '<' : '\"', angled ? '>' : '\"', false);

    if (parsed != NULL && parsed->type == PP_TOK_STRING_LIT)
    {
        char *filename = parsed->string.value; // no need to save the len, no NUL in file names (if there are NUL in file names, you are an horrible person)

        parsed->type = PP_TOK_HEADER;
        parsed->header.name = filename;
        parsed->header.is_angled = angled;
    }

    context_free(lcontext);
    return parsed;
}

// parse a macro name
static struct pp_token_s *parse_macro_name(context_t *context, struct pp_tokstream_s const *stream, size_t *restrict n)
{
    // short circuit
    if (!stream->is_line_define || stream->tokens_given != 2)
    {
        *n = 0;
        return NULL;
    }

    // parse an identifier
    struct pp_token_s *parsed = parse_identifier(context, stream, n);
    if (parsed != NULL)
    {
        // converting token
        char *name = parsed->name;
        parsed->type = PP_TOK_MACRO_NAME;
        parsed->macro_name.name = name;
        parsed->macro_name.is_function = false;

        // checking if function-like
        if (stream->current_line->content[stream->cursor + *n] == '(')
        {
            parsed->macro_name.is_function = true;
            (*n)++;
        }
    }

    return parsed;
}

static const parsing_fun_ptr_t PARSING_FUNCTIONS[] = {
    &parse_whitespace,
    &parse_macro_name, // <- MUST be before identifier, so it take precedence
    &parse_identifier,
    &parse_pp_number,
    &parse_header_name, // <- MUST be before string_literal, so it take precedence
    &parse_string_literal,
    &parse_char_literal,
    &parse_punctuator,
    &parse_comment,
    NULL};

// --- MULTILINE COMMENTS SPECIAL RULE ---

static bool parse_multiline_comment(context_t *context, pp_tokstream_t *stream)
{
    context_t *lcontext = context_new(context, TOKENIZER_CONTEXT_MULTILINE);
    stream->cursor += 2; // jump the "/*"
    do
    {
        for (; stream->current_line->content[stream->cursor] != '\0'; stream->cursor++)
            if (stream->current_line->content[stream->cursor] == '*' &&
                stream->current_line->content[stream->cursor + 1] == '/')
            {
                stream->cursor += 2; // jump the "*/"
                return true;
            }

        // line exausted, go to the next without updating flags
        line_free(stream->current_line);
        stream->current_line = linestream_get(lcontext, stream->source);
        stream->cursor = 0;
    } while (stream->current_line != NULL);

    // file ended inside multiline!
    return false;
}

// --- TOKEN PARSING MANAGER ---

struct pp_token_s *pp_tokstream_get(context_t *context, pp_tokstream_t *stream)
{
    // return ungetted token
    if (stream->ungetten_token != NULL)
    {
        struct pp_token_s *new_token = stream->ungetten_token;
        stream->ungetten_token = NULL;
        return new_token;
    }

    // return delayed token
    if (stream->delayed_directive_end != NULL)
    {
        struct pp_token_s *new_token = stream->delayed_directive_end;
        stream->delayed_directive_end = NULL;
        return new_token;
    }

    context_t *lcontext = context_new(context, TOKENIZER_CONTEXT_GETTING);

    // the token parsed
    struct pp_token_s *new_token = NULL;
    // where the last token parsed started
    struct bookmark_s parsed_start = bookmark_new(NULL, 0, 0);

    do
    {
        // checking if we need a new line
        if (stream->current_line == NULL)
        {
            // get a new line from the source
            stream->current_line = linestream_get(context, stream->source);
            if (stream->current_line == NULL)
                return NULL; // source exausted

            stream->cursor = 0;
            stream->tokens_given = 0;

            stream->is_line_directive = false;
            stream->is_line_include = false;
            stream->is_line_define = false;
        }
        // multiline comments are the only token that can span multiple lines, so they get a special treatement
        if (stream->current_line->content[stream->cursor] == '/' &&
            stream->current_line->content[stream->cursor + 1] == '*')
        {
            if (!parse_multiline_comment(lcontext, stream))
            {
                // error in parsing multiline comment
                new_token = malloc(sizeof(struct pp_token_s));
                if (new_token == NULL)
                    log_error(lcontext, TOKENIZER_MALLOC_FAIL_TOKEN);

                new_token->type = PP_TOK_ERROR;

                new_token->error.msg = malloc(strlen(TOKENIZER_EOF_MULTILINE) + 1);
                if (new_token->error.msg == NULL)
                    log_error(lcontext, TOKENIZER_MALLOC_FAIL_ERROR);
                strcpy(new_token->error.msg, TOKENIZER_EOF_MULTILINE);

                new_token->error.severity = LOG_ERROR;

                break;
            }
        }
        else
        {
            // save the mark
            parsed_start = line_mark(stream->current_line, stream->cursor);

            // Greedy parsing: try all the parsing functions and chose
            // the one that take the most chars
            size_t best_chars = 0;
            for (size_t i = 0; PARSING_FUNCTIONS[i] != NULL; i++)
            {
                // try parse a type of token
                size_t try_chars;
                struct pp_token_s *try_token = PARSING_FUNCTIONS[i](lcontext, stream, &try_chars);
                if (try_chars > best_chars)
                {
                    // found a longer token, destroing the other
                    if (new_token != NULL)
                        pp_tok_free(new_token);
                    new_token = try_token;
                    best_chars = try_chars;
                }
            }
            stream->cursor += best_chars; // update the cursor
            // if the token is non-null the cycle will break

            if (best_chars == 0)
            {
                // all tokenization failed, collecting stray char
                char stray = stream->current_line->content[stream->cursor++];

                // creating error token
                new_token = malloc(sizeof(struct pp_token_s));
                if (new_token == NULL)
                    log_error(lcontext, TOKENIZER_MALLOC_FAIL_TOKEN);

                new_token->type = PP_TOK_ERROR;
                new_token->error.severity = LOG_ERROR;

                new_token->error.msg = malloc(strlen(TOKENIZER_STRAY_CHAR_OPEN) +
                                              strlen(CHARESCAPE(stray)) +
                                              strlen(TOKENIZER_STRAY_CHAR_CLOSE) + 1);
                if (new_token->error.msg == NULL)
                    log_error(lcontext, TOKENIZER_MALLOC_FAIL_ERROR);
                strcat(strcat(strcpy(new_token->error.msg, TOKENIZER_STRAY_CHAR_OPEN), CHARESCAPE(stray)), TOKENIZER_STRAY_CHAR_CLOSE);
            }
        }

        if (stream->cursor == strlen(stream->current_line->content))
        {
            // line is completed
            if (stream->is_line_directive)
            {
                // deciding if we want to give it now or after this token
                struct pp_token_s **const dir_end_token = (new_token == NULL) ? (&new_token) : (&stream->delayed_directive_end);

                *dir_end_token = malloc(sizeof(struct pp_token_s));
                if (*dir_end_token == NULL)
                    log_error(lcontext, TOKENIZER_MALLOC_FAIL_TOKEN);
                (*dir_end_token)->type = PP_TOK_DIRECTIVE_STOP;
                (*dir_end_token)->mark = line_mark(stream->current_line, stream->cursor);
            }

            line_free(stream->current_line);
            stream->current_line = NULL;
        }
    } while (new_token == NULL); // break at the first non-null token found

    // mark the token
    if (new_token->type != PP_TOK_DIRECTIVE_STOP)
        new_token->mark = parsed_start; // TODO: what if some tokens are in different position? like string errors?

    // directives and include detecting
    if (stream->tokens_given == 0 // first token
        && new_token->type == PP_TOK_PUNCTUATOR && new_token->punc_kind == PUNC_STRINGIZE)
    {
        stream->is_line_directive = true;
        new_token->type = PP_TOK_DIRECTIVE_START; // changing token type
    }
    if (stream->is_line_directive && stream->tokens_given == 1 // first token of a directive
        && new_token->type == PP_TOK_IDENTIFIER)
    {
        if (strcmp(new_token->name, "include") == 0)
            stream->is_line_include = true;
        else if (strcmp(new_token->name, "define") == 0)
            stream->is_line_define = true;
    }

    // count the token
    stream->tokens_given++;

    context_free(lcontext);
    return new_token;
}

void pp_tokstream_unget(pp_tokstream_t *stream, struct pp_token_s *token)
{
#ifdef CHECK_UNGETTOKEN
    if (stream->ungetten_token != NULL)
        log_error(NULL, TOKENIZER_UNGET_FULL);
#endif
    stream->ungetten_token = token;
}