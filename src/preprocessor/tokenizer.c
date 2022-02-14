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
    size_t tokens_given;                 // number of tokens extracted (for include checking)
    bool is_line_directive;              // mark if this line is a directive (first non-whitespace character is #)
    bool is_line_include;                // mark if this line is an include (directive == true, first token after DIRECTIVE_START is an `include` identifier)
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

void pp_tokstream_close(pp_tokstream_t *stream, bool recursive_close)
{
    if (recursive_close)
        linestream_close(stream->source, recursive_close);
    if (stream->current_line != NULL)
        line_free(stream->current_line);
    free(stream);
}

// --- TOKEN END OF LIFE ---

void pp_tok_free(struct pp_token_s *token)
{
    if (token->type == PP_TOK_IDENTIFIER ||
        token->type == PP_TOK_PP_NUMBER ||
        token->type == PP_TOK_STRING_LIT)
        free(token->content);
    if (token->type == PP_TOK_HEADER)
        free(token->header.name);
    if (token->type == PP_TOK_ERROR)
        free(token->error.msg);
    free(token);
}

// --- TOKEN COMPARATION ---

bool pp_tok_cmp(struct pp_token_s *a, struct pp_token_s *b)
{
    if (a->type != b->type)
        return false;

    switch (a->type)
    {
    case PP_TOK_IDENTIFIER:
    case PP_TOK_PP_NUMBER:
    case PP_TOK_STRING_LIT:
        return strcmp(a->content, b->content) == 0;

    case PP_TOK_CHAR_CONST:
        return a->char_value == b->char_value;

    case PP_TOK_HEADER:
        return a->header.is_angled == b->header.is_angled && strcmp(a->header.name, b->header.name) == 0;

    case PP_TOK_PUNCTUATOR:
        return a->punc_kind == b->punc_kind;

    case PP_TOK_DIRECTIVE_START:
    case PP_TOK_DIRECTIVE_STOP:
        return true; //contentless

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

/* TODO: both parse_identifier and parse_pp_number use a dinamically allocated buffer 
         of the same lenght of the line. maybe use open_memstream?*/

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
    new_token->content = name;

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

    // allocate space for the content
    char *content = malloc(strlen(stream->current_line->content) - stream->cursor + 1);
    if (content == NULL)
        log_error(lcontext, TOKENIZER_MALLOC_FAIL_PP_NUMBER);

    // count copied chars
    size_t numlen = 0;

    // consume optional perion
    if (stream->current_line->content[stream->cursor + numlen] == '.')
        content[numlen++] = '.';

    // check required digit
    if (!isdigit(stream->current_line->content[stream->cursor + numlen]))
    {
        *n = 0;
        return NULL;
    }

    do
    {
        content[numlen] = stream->current_line->content[stream->cursor + numlen];
        numlen++;
    } while (stream->current_line->content[stream->cursor + numlen] != '\0' &&              // if the line is still going and
             (is_identifier_char(stream->current_line->content[stream->cursor + numlen]) || // next char is digit, or alpha, or underscore
              stream->current_line->content[stream->cursor + numlen] == '.' ||              // or a period
              (is_sign(stream->current_line->content[stream->cursor + numlen]) &&           // ora a sign, but only if
               is_exp_start(stream->current_line->content[stream->cursor + numlen - 1])))); // preceeded by a exponent start

    // 0-terminate content
    content[numlen] = '\0';

    // shrink name
    char *new_content = realloc(content, numlen + 1);
    if (new_content == NULL)
        log_warning(lcontext, TOKENIZER_MALLOC_FAIL_SHRINKPP_NUMBER); // continue with unshrinked content
    else
        content = new_content;

    // create token
    struct pp_token_s *new_token = malloc(sizeof(struct pp_token_s));
    if (new_token == NULL)
        log_error(lcontext, TOKENIZER_MALLOC_FAIL_TOKEN);

    new_token->type = PP_TOK_PP_NUMBER;
    new_token->content = content;

    context_free(lcontext);

    *n = numlen;
    return new_token;
}

// TODO: parse string literals
// TODO: parse char consts
// TODO: parst punctuators
// TODO: parse comments
// TODO: parse header names

static const parsing_fun_ptr_t PARSING_FUNCTIONS[] = {
    &parse_whitespace,
    &parse_identifier,
    &parse_pp_number,
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
    context_t *lcontext = context_new(context, TOKENIZER_CONTEXT_GETTING);

    // the token parsed
    struct pp_token_s *new_token = NULL;

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
        }

        // multiline comments are the only token that can span multiple lines, so they get a special treatement
        if (stream->current_line->content[stream->cursor] == '/' &&
            stream->current_line->content[stream->cursor + 1] == '*')
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

        if (stream->cursor == strlen(stream->current_line->content))
        {
            // line is completed
            if (stream->is_line_directive)
                new_token->type = PP_TOK_DIRECTIVE_STOP;

            line_free(stream->current_line);
            stream->current_line = NULL;
        }
        else if (best_chars == 0)
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

    } while (new_token == NULL); // break at the first non-null token found

    // TODO: check if newline is a directive and an include
    // TODO: bookmark the token

    // count the token
    stream->tokens_given++;

    context_free(lcontext);
    return new_token;
}