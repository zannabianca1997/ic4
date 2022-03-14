/**
 * @file directives.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Directive parser
 * @version 0.1
 * @date 2022-03-07
 *
 * Break a token stream into a stream of directives
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "directives.h"

#include "../misc/queue.h"
#include "../misc/log/log.h"

#include "messages.cat.h"

#if __GNUC__
#define ATTR_UNUSED __attribute((unused))
#else
#define ATTR_UNUSED
#endif

// -- directive end of life
static void _pp_tok_free_wrapped(void *tok) { pp_tok_free((struct pp_token_s *)tok); }
void directive_free(struct pp_directive_s *directive)
{
    switch (directive->type)
    {
    case PP_DIRECTIVE_LINE_CTRL:
        if (directive->line_ctrl.need_macros)
        {
            for (size_t i = 0; i < directive->line_ctrl.nargs; i++)
                pp_tok_free(directive->line_ctrl.args[i]);
            free(directive->line_ctrl.args);
        }
        else
            free(directive->line_ctrl.file_name);
        break;

    case PP_DIRECTIVE_INCLUDE:
        if (directive->include.need_macros)
        {
            for (size_t i = 0; i < directive->include.nargs; i++)
                pp_tok_free(directive->include.args[i]);
            free(directive->include.args);
        }
        else
            free(directive->include.file_name);
        break;

    case PP_DIRECTIVE_DEFINE:
        free(directive->define.macro_name);
        for (size_t i = 0; i < directive->define.nargs; i++)
            free(directive->define.args[i]);
        free(directive->define.args);

        for (size_t i = 0; i < directive->define.ntokens; i++)
            pp_tok_free(directive->define.tokens[i]);
        free(directive->define.tokens);
        break;

    case PP_DIRECTIVE_UNDEF:
        free(directive->undefined_name);
        break;

    case PP_DIRECTIVE_ERROR:
        free(directive->error.msg);
        break;

    case PP_DIRECTIVE_IF:
    case PP_DIRECTIVE_ELIF:
    case PP_DIRECTIVE_PRAGMA:
    case PP_DIRECTIVE_EMIT:
        for (size_t i = 0; i < directive->nargs; i++)
            pp_tok_free(directive->args[i]);
        free(directive->args);
        break;

    case PP_DIRECTIVE_IFDEF:
        free(directive->ifdef.macro_name);
        break;

    case PP_DIRECTIVE_ELSE:
    case PP_DIRECTIVE_ENDIF:
        break;
    }

    free(directive);
}
static void _directive_free_wrapped(void *dir) { directive_free((struct pp_directive_s *)dir); }

// -- directive stream

/**
 * @brief Contain a stream of directives
 *
 */
struct directive_stream_s
{
    pp_tokstream_t *source;
    queue_t *errors;

    size_t if_depth, max_depth;
    bool *else_emitted;
};

directive_stream_t *directive_stream_open(context_t *context, pp_tokstream_t *source)
{
    directive_stream_t *res = malloc(sizeof(directive_stream_t));
    if (res == NULL)
        log_error(context_new(context, DIRECTIVES_CONTEXT_OPENING), DIRECTIVES_MALLOC_FAIL_OPEN);

    res->source = source;
    res->errors = queue_new();
    if (res->errors == NULL)
        log_error(context_new(context, DIRECTIVES_CONTEXT_OPENING), DIRECTIVES_QUEUE_FAIL_CREATING);

    res->if_depth = 0;
    res->max_depth = 0;
    res->else_emitted = NULL;

    return res;
}

void directive_stream_close(directive_stream_t *stream, bool recursive_close)
{
    if (recursive_close)
        pp_tokstream_close(stream->source, true);

    queue_free(stream->errors, &_directive_free_wrapped);
    free(stream);
}

// -- parsing

/**
 * @brief Create an error directive
 *
 * @param lcontext the context of the directive creation
 * @param mark the position of the error
 * @param level the severity of the error
 * @param msg the message of the error
 * @return struct pp_directive_s * the created directive
 */
static inline struct pp_directive_s *make_error_directive(context_t *lcontext, struct bookmark_s mark, enum loglevel_e level, const char *msg, struct pp_directive_s *directive)
{
    directive->type = PP_DIRECTIVE_ERROR;
    directive->mark = mark;
    directive->error.severity = level;

    directive->error.msg = malloc(strlen(msg) + 1);
    if (directive->error.msg == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_STRDUP);
    strcpy(directive->error.msg, msg);

    return directive;
}

/**
 * @brief get the next token.
 *
 * Used as a filter to collect the errors
 *
 * @param context
 * @param stream
 * @return struct pp_token_s*
 */
static inline struct pp_token_s *next_token(context_t *context, struct directive_stream_s *stream)
{
    struct pp_token_s *token = pp_tokstream_get(context, stream->source);
    while (token != NULL && token->type == PP_TOK_ERROR)
    {
        struct pp_directive_s *directive = malloc(sizeof(struct pp_directive_s));
        if (directive == NULL)
            log_error(context, DIRECTIVES_MALLOC_FAIL_DIRECTIVE);
        make_error_directive(context, token->mark, token->error.severity, token->error.msg, directive);

        if (!queue_push(stream->errors, directive))
            log_error(context, DIRECTIVES_QUEUE_ADD_ERROR);

        pp_tok_free(token), token = pp_tokstream_get(context, stream->source);
    }
    return token;
}

static void make_linectrl_directive(context_t *context, struct bookmark_s mark, queue_t *args, struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_LINECTRL);

    // no arguments
    if (queue_is_empty(args))
    {
        make_error_directive(lcontext, directive_end, LOG_ERROR, DIRECTIVES_LINECTRL_NOARG, new_directive);
        context_free(lcontext);
        return;
    }

    struct pp_token_s *linenum_tok = queue_pop(args);
    struct pp_token_s *filename_tok = queue_pop(args);

    bool need_macro = !queue_is_empty(args);
    size_t linenum;
    char *filename = NULL;

    if (!need_macro && linenum_tok->type == PP_TOK_PP_NUMBER)
    {
        char *parsed_end;
        unsigned long long parsed_linenum = strtoull(linenum_tok->name, &parsed_end, 10);
        if ((size_t)(parsed_end - linenum_tok->name) == strlen(linenum_tok->name))
        {
            if (parsed_linenum < SIZE_MAX)
                linenum = (size_t)parsed_linenum;
            else
            {
                make_error_directive(lcontext, mark, LOG_ERROR, DIRECTIVES_LINECTRL_LINENUM_TOO_HIGH, new_directive);
                context_free(lcontext);
                pp_tok_free(linenum_tok);
                if (filename_tok != NULL)
                    pp_tok_free(filename_tok);
                return;
            }
        }
        else
            need_macro = true;
    }
    else
        need_macro = true;

    if (!need_macro && filename_tok != NULL)
    {
        if (filename_tok->type == PP_TOK_STRING_LIT)
        {
            // this will cut the string to the first NUL, if present.
            // I choose to ignore the problem. No sane filesystem have NUL in file paths
            filename = malloc(strlen(filename_tok->string.value) + 1);
            if (filename == NULL)
                log_error(lcontext, DIRECTIVES_MALLOC_FAIL_STRDUP);
            strcpy(filename, filename_tok->string.value);
        }
        else
            need_macro = true;
    }

    new_directive->type = PP_DIRECTIVE_LINE_CTRL;
    new_directive->mark = mark;
    new_directive->line_ctrl.need_macros = need_macro;
    if (need_macro)
    {
        if (filename_tok != NULL)
            new_directive->line_ctrl.nargs = 2 + queue_len(args);
        else
            new_directive->line_ctrl.nargs = 1;

        new_directive->line_ctrl.args = malloc(new_directive->line_ctrl.nargs * sizeof(struct pp_token_s *));
        if (new_directive->line_ctrl.args == NULL)
            log_error(lcontext, DIRECTIVES_MALLOC_FAIL_LINECTRL_TOKENS);

        // collecting first token
        new_directive->line_ctrl.args[0] = linenum_tok;
        // collecting second token
        if (filename_tok != NULL)
            new_directive->line_ctrl.args[1] = filename_tok;
        // collecting rest of tokens
        size_t idx = 2;
        while (!queue_is_empty(args))
            new_directive->line_ctrl.args[idx++] = queue_pop(args);

        // DO NOT FREE linenum_tok and filename_tok => they are now into the directive!
    }
    else
    {
        pp_tok_free(linenum_tok);
        if (filename_tok != NULL)
            pp_tok_free(filename_tok);

        new_directive->line_ctrl.line_num = linenum;
        new_directive->line_ctrl.file_name = filename;
    }

    context_free(lcontext);
}

static void make_define_directive(context_t *context, struct bookmark_s mark, queue_t *args, struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_DEFINE);

    // no arguments
    if (queue_is_empty(args))
    {
        make_error_directive(lcontext, directive_end, LOG_ERROR, DIRECTIVES_DEFINE_ERROR_NAME, new_directive);
        context_free(lcontext);
        return;
    }

    // read macro name
    struct pp_token_s *macro_name_tok = queue_pop(args);
    if (macro_name_tok->type != PP_TOK_MACRO_NAME)
    {
        make_error_directive(lcontext, macro_name_tok->mark, LOG_ERROR, DIRECTIVES_DEFINE_ERROR_NAME, new_directive);
        pp_tok_free(macro_name_tok);
        context_free(lcontext);
        return;
    }

    // reading args if needed
    if (macro_name_tok->macro_name.is_function)
    {
        struct pp_token_s *arg = queue_pop(args);
        if (arg == NULL)
        {
            make_error_directive(lcontext, directive_end, LOG_ERROR, DIRECTIVES_ERROR_IDENT_OR_LPAR_EXPECTED, new_directive);
            context_free(lcontext);
            return;
        }

        if (arg->type == PP_TOK_PUNCTUATOR && arg->punc_kind == PUNC_PAR_RIGHT)
            new_directive->define.nargs = 0;
        else
        {
            queue_t *macro_args = queue_new();
            if (macro_args == NULL)
                log_error(lcontext, DIRECTIVES_QUEUE_FAIL_CREATING);
            while (true)
            {
                if (arg->type != PP_TOK_IDENTIFIER)
                {
                    make_error_directive(lcontext, arg->mark, LOG_ERROR, DIRECTIVES_ERROR_IDENTIFIER_EXPECTED, new_directive);
                    context_free(lcontext);
                    return;
                }

                char *argname = malloc(strlen(arg->name) + 1);
                if (argname == NULL)
                    log_error(lcontext, DIRECTIVES_MALLOC_FAIL_STRDUP);
                strcpy(argname, arg->name);
                if (!queue_push(macro_args, argname))
                    log_error(lcontext, DIRECTIVES_QUEUE_ADD_ARG);

                pp_tok_free(arg), arg = queue_pop(args);

                if (arg == NULL)
                {
                    make_error_directive(lcontext, directive_end, LOG_ERROR, DIRECTIVES_ERROR_COMMA_OR_LPAR_EXPECTED, new_directive);
                    context_free(lcontext);
                    return;
                }

                if (arg->type == PP_TOK_PUNCTUATOR && arg->punc_kind == PUNC_PAR_RIGHT)
                    break;
                if (!(arg->type == PP_TOK_PUNCTUATOR && arg->punc_kind == PUNC_COMMA))
                {
                    make_error_directive(lcontext, arg->mark, LOG_ERROR, DIRECTIVES_ERROR_COMMA_OR_LPAR_EXPECTED, new_directive);
                    context_free(lcontext);
                    return;
                }

                pp_tok_free(arg), arg = queue_pop(args);
            }
            pp_tok_free(arg);

            // writing down args
            new_directive->define.nargs = queue_len(macro_args);
            new_directive->define.args = malloc(new_directive->define.nargs * sizeof(char *));
            if (new_directive->define.args == NULL)
                log_error(lcontext, DIRECTIVES_MALLOC_FAIL_DEFINE_ARGS);
            size_t idx = 0;
            while (!queue_is_empty(macro_args))
                new_directive->define.args[idx++] = queue_pop(macro_args);
        }
    }

    // setting up macro type and name
    new_directive->type = PP_DIRECTIVE_DEFINE;
    new_directive->mark = mark;

    new_directive->define.macro_name = malloc(strlen(macro_name_tok->macro_name.name) + 1);
    if (new_directive->define.macro_name == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_STRDUP);
    strcpy(new_directive->define.macro_name, macro_name_tok->macro_name.name);

    new_directive->define.is_function = macro_name_tok->macro_name.is_function;

    pp_tok_free(macro_name_tok);

    // collecting remaining tokens
    new_directive->define.ntokens = queue_len(args);
    new_directive->define.tokens = malloc(new_directive->define.ntokens * sizeof(struct pp_token_s *));
    size_t idx = 0;
    while (!queue_is_empty(args))
        new_directive->define.tokens[idx++] = queue_pop(args);
    context_free(lcontext);
}

static void make_undef_directive(context_t *context, struct bookmark_s mark, queue_t *args, struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_UNDEF);

    if (queue_is_empty(args))
    {
        make_error_directive(lcontext, directive_end, LOG_ERROR, DIRECTIVES_ERROR_IDENTIFIER_EXPECTED, new_directive);
        context_free(lcontext);
        return;
    }

    struct pp_token_s *identifier = queue_pop(args);
    if (identifier->type != PP_TOK_IDENTIFIER)
    {
        make_error_directive(lcontext, identifier->mark, LOG_ERROR, DIRECTIVES_ERROR_IDENTIFIER_EXPECTED, new_directive);
        context_free(lcontext);
        return;
    }

    if (!queue_is_empty(args))
    {
        struct pp_token_s *first_unwanted = queue_pop(args);
        make_error_directive(lcontext, first_unwanted->mark, LOG_ERROR, DIRECTIVES_ERROR_EOL_EXPECTED, new_directive);
        pp_tok_free(first_unwanted);
        pp_tok_free(identifier);
        context_free(lcontext);
        return;
    }

    new_directive->type = PP_DIRECTIVE_UNDEF;
    new_directive->mark = mark;
    new_directive->undefined_name = malloc(strlen(identifier->name) + 1);
    if (new_directive->undefined_name == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_STRDUP);
    strcpy(new_directive->undefined_name, identifier->name);

    pp_tok_free(identifier);
    context_free(lcontext);
}

static void make_include_directive(context_t *context, struct bookmark_s mark, queue_t *args, ATTR_UNUSED struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_DEFINE);

    new_directive->type = PP_DIRECTIVE_INCLUDE;
    new_directive->mark = mark;

    if (queue_len(args) == 1)
    {
        struct pp_token_s *filename = queue_pop(args);
        if (filename->type == PP_TOK_HEADER)
        {
            new_directive->include.need_macros = false;
            new_directive->include.is_angled = filename->header.is_angled;

            new_directive->include.file_name = malloc(strlen(filename->header.name) + 1);
            if (new_directive->include.file_name == NULL)
                log_error(lcontext, DIRECTIVES_MALLOC_FAIL_STRDUP);
            strcpy(new_directive->include.file_name, filename->header.name);

            pp_tok_free(filename);
            context_free(lcontext);
            return;
        }

        queue_push(args, filename); // put back the only token
    }

    new_directive->include.need_macros = true;
    new_directive->include.nargs = queue_len(args);
    new_directive->include.args = malloc(new_directive->include.nargs * sizeof(struct pp_token_s *));
    if (new_directive->include.args == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_INCLUDE_TOKENS);
    size_t idx = 0;
    while (!queue_is_empty(args))
        new_directive->include.args[idx++] = queue_pop(args);

    context_free(lcontext);
}

static void make_directive_from_args(context_t *context, queue_t *args, struct pp_directive_s *new_directive)
{
    new_directive->nargs = queue_len(args);
    new_directive->args = malloc(new_directive->nargs * sizeof(struct pp_token_s *));
    if (new_directive->args == NULL)
        log_error(context, DIRECTIVES_MALLOC_FAIL_ARGS);
    size_t idx = 0;
    while (!queue_is_empty(args))
        new_directive->args[idx++] = queue_pop(args);
}

static void make_if_directive(context_t *context, struct bookmark_s mark, queue_t *args, ATTR_UNUSED struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_IF);

    new_directive->type = PP_DIRECTIVE_IF;
    new_directive->mark = mark;
    make_directive_from_args(lcontext, args, new_directive);

    context_free(lcontext);
}

static void make_ifdef_directive(context_t *context, struct bookmark_s mark, queue_t *args, bool negated, struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_IFDEF);

    if (queue_is_empty(args))
    {
        make_error_directive(lcontext, directive_end, LOG_ERROR, DIRECTIVES_ERROR_IDENTIFIER_EXPECTED, new_directive);
        context_free(lcontext);
        return;
    }

    struct pp_token_s *identifier = queue_pop(args);
    if (identifier->type != PP_TOK_IDENTIFIER)
    {
        make_error_directive(lcontext, identifier->mark, LOG_ERROR, DIRECTIVES_ERROR_IDENTIFIER_EXPECTED, new_directive);
        context_free(lcontext);
        return;
    }

    if (!queue_is_empty(args))
    {
        struct pp_token_s *first_unwanted = queue_pop(args);
        make_error_directive(lcontext, first_unwanted->mark, LOG_ERROR, DIRECTIVES_ERROR_EOL_EXPECTED, new_directive);
        pp_tok_free(first_unwanted);
        pp_tok_free(identifier);
        context_free(lcontext);
        return;
    }

    new_directive->type = PP_DIRECTIVE_IFDEF;
    new_directive->mark = mark;
    new_directive->ifdef.negated = negated;
    new_directive->ifdef.macro_name = malloc(strlen(identifier->name) + 1);
    if (new_directive->ifdef.macro_name == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_STRDUP);
    strcpy(new_directive->ifdef.macro_name, identifier->name);

    pp_tok_free(identifier);
    context_free(lcontext);
}

static void make_elif_directive(context_t *context, struct bookmark_s mark, queue_t *args, ATTR_UNUSED struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_ELIF);

    new_directive->type = PP_DIRECTIVE_ELIF;
    new_directive->mark = mark;
    make_directive_from_args(lcontext, args, new_directive);

    context_free(lcontext);
}

static void make_else_directive(context_t *context, struct bookmark_s mark, queue_t *args, ATTR_UNUSED struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_ELSE);

    if (queue_len(args) != 0)
    {
        struct pp_token_s *unwanted = queue_pop(args);
        make_error_directive(lcontext, unwanted->mark, LOG_ERROR, DIRECTIVES_ELSE_ARGS, new_directive);
        pp_tok_free(unwanted);
        return;
    }
    new_directive->type = PP_DIRECTIVE_ELSE;
    new_directive->mark = mark;

    context_free(lcontext);
}

static void make_endif_directive(context_t *context, struct bookmark_s mark, queue_t *args, ATTR_UNUSED struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_ENDIF);

    if (queue_len(args) != 0)
    {
        struct pp_token_s *unwanted = queue_pop(args);
        make_error_directive(lcontext, unwanted->mark, LOG_ERROR, DIRECTIVES_ENDIF_ARGS, new_directive);
        pp_tok_free(unwanted);
        return;
    }
    new_directive->type = PP_DIRECTIVE_ENDIF;
    new_directive->mark = mark;

    context_free(lcontext);
}

static void make_pragma_directive(context_t *context, struct bookmark_s mark, queue_t *args, ATTR_UNUSED struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_PRAGMA);

    new_directive->type = PP_DIRECTIVE_PRAGMA;
    new_directive->mark = mark;
    make_directive_from_args(lcontext, args, new_directive);

    context_free(lcontext);
}

static void make_error_directive_from_args(context_t *context, struct bookmark_s mark, queue_t *args, struct bookmark_s directive_end, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_ERROR);
    log_error(lcontext, "Unimplemented");
    context_free(lcontext);
}

static void parse_directive(context_t *context, directive_stream_t *stream, struct pp_directive_s *new_directive)
{
    // collect name
    struct pp_token_s *name_token = next_token(context, stream);

    // name token cannot be null cause every directive must be closed
    if (name_token->type == PP_TOK_DIRECTIVE_STOP)
    {
        // empty directive... GCC accept it, we emit a warning
        make_error_directive(context, name_token->mark, LOG_WARNING, DIRECTIVES_ERROR_NAME, new_directive);
        pp_tok_free(name_token);
        return;
    }
    else if (name_token->type != PP_TOK_IDENTIFIER)
    {
        // directive without a name...
        make_error_directive(context, name_token->mark, LOG_ERROR, DIRECTIVES_ERROR_NAME, new_directive);
        pp_tok_free(name_token);
        return;
    }

    // collecting args
    queue_t *args = queue_new();
    if (args == NULL)
        log_error(context, DIRECTIVES_QUEUE_FAIL_CREATING);
    struct bookmark_s directive_end;
    {
        struct pp_token_s *token = next_token(context, stream);
        while (token->type != PP_TOK_DIRECTIVE_STOP)
        {
            if (!queue_push(args, token))
                log_error(context, DIRECTIVES_QUEUE_ADD_DIRECTIVE_TOKEN);
            token = next_token(context, stream);
        }
        directive_end = token->mark;
        pp_tok_free(token);
    }

    switch (name_token->name[0])
    {
    case 'l':
        if (strcmp(name_token->name, "line") == 0)
            make_linectrl_directive(context, name_token->mark, args, directive_end, new_directive);
        else
            make_error_directive(context, name_token->mark, LOG_ERROR, DIRECTIVES_ERROR_UNKNOW, new_directive);
        break;

    case 'd':
        if (strcmp(name_token->name, "define") == 0)
            make_define_directive(context, name_token->mark, args, directive_end, new_directive);
        else
            make_error_directive(context, name_token->mark, LOG_ERROR, DIRECTIVES_ERROR_UNKNOW, new_directive);
        break;

    case 'u':
        if (strcmp(name_token->name, "undef") == 0)
            make_undef_directive(context, name_token->mark, args, directive_end, new_directive);
        else
            make_error_directive(context, name_token->mark, LOG_ERROR, DIRECTIVES_ERROR_UNKNOW, new_directive);
        break;

    case 'i':
        if (strcmp(name_token->name, "include") == 0)
            make_include_directive(context, name_token->mark, args, directive_end, new_directive);
        else if (strcmp(name_token->name, "if") == 0)
            make_if_directive(context, name_token->mark, args, directive_end, new_directive);
        else if (strcmp(name_token->name, "ifdef") == 0)
            make_ifdef_directive(context, name_token->mark, args, false, directive_end, new_directive);
        else if (strcmp(name_token->name, "ifndef") == 0)
            make_ifdef_directive(context, name_token->mark, args, true, directive_end, new_directive);
        else
            make_error_directive(context, name_token->mark, LOG_ERROR, DIRECTIVES_ERROR_UNKNOW, new_directive);
        break;

    case 'e':
        if (strcmp(name_token->name, "else") == 0)
            make_else_directive(context, name_token->mark, args, directive_end, new_directive);
        else if (strcmp(name_token->name, "endif") == 0)
            make_endif_directive(context, name_token->mark, args, directive_end, new_directive);
        else if (strcmp(name_token->name, "elif") == 0)
            make_elif_directive(context, name_token->mark, args, directive_end, new_directive);
        else if (strcmp(name_token->name, "error") == 0)
            make_error_directive_from_args(context, name_token->mark, args, directive_end, new_directive);
        else
            make_error_directive(context, name_token->mark, LOG_ERROR, DIRECTIVES_ERROR_UNKNOW, new_directive);
        break;

    case 'p':
        if (strcmp(name_token->name, "pragma") == 0)
            make_pragma_directive(context, name_token->mark, args, directive_end, new_directive);
        else
            make_error_directive(context, name_token->mark, LOG_ERROR, DIRECTIVES_ERROR_UNKNOW, new_directive);
        break;

    default:
        make_error_directive(context, name_token->mark, LOG_ERROR, DIRECTIVES_ERROR_UNKNOW, new_directive);
        break;
    }

    // freeing used tokens
    pp_tok_free(name_token);
    queue_free(args, &_pp_tok_free_wrapped); // if not already consumed
    return;
}

static void parse_running_text(context_t *context, directive_stream_t *stream, struct pp_directive_s *new_directive)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_FREE_TEXT);

    queue_t *collected_tokens = queue_new();
    if (collected_tokens == NULL)
        log_error(context, DIRECTIVES_QUEUE_FAIL_CREATING);

    struct pp_token_s *token;
    while ((token = next_token(lcontext, stream)) != NULL && token->type != PP_TOK_DIRECTIVE_START)
    {
        if (!queue_push(collected_tokens, token))
            log_error(lcontext, DIRECTIVES_QUEUE_ADD_FREE_TOKEN);
    }
    pp_tokstream_unget(stream->source, token); // unget the PP_TOK_DIRECTIVE_START

    // creating emit directive
    new_directive->type = PP_DIRECTIVE_EMIT;

    new_directive->nargs = queue_len(collected_tokens);
    new_directive->args = malloc(new_directive->nargs * sizeof(struct pp_token_s *));
    if (new_directive->args == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_EMIT_TOKENS);

    for (size_t idx; !queue_is_empty(collected_tokens); idx++)
        new_directive->args[idx] = queue_pop(collected_tokens);
    queue_free(collected_tokens, NULL);

    new_directive->mark = new_directive->args[0]->mark;

    context_free(lcontext);
}

struct pp_directive_s *directive_stream_get(context_t *context, directive_stream_t *stream)
{
    // emptying the error queue first
    if (!queue_is_empty(stream->errors))
        return queue_pop(stream->errors);

    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_GETTING);
    struct pp_token_s *token = next_token(lcontext, stream);
    if (token == NULL)
    {
        context_free(lcontext);
        return NULL;
    }

    struct pp_directive_s *new_directive = malloc(sizeof(struct pp_directive_s));
    if (new_directive == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_DIRECTIVE);

    // checking for next token
    if (token->type == PP_TOK_DIRECTIVE_START)
        parse_directive(lcontext, stream, new_directive);
    else
    {
        pp_tokstream_unget(stream->source, token); // token is part of the text
        parse_running_text(lcontext, stream, new_directive);
    }

    if (new_directive->type == PP_DIRECTIVE_IF || new_directive->type == PP_DIRECTIVE_IFDEF)
    {
        stream->if_depth++;
        if (stream->if_depth > stream->max_depth)
        {
            bool *new_elses = realloc(stream->else_emitted, stream->if_depth * sizeof(bool));
            if (new_elses == NULL)
                log_error(lcontext, DIRECTIVES_MALLOC_FAIL_ELSE_TRACK);
            stream->else_emitted = new_elses;
            stream->max_depth = stream->if_depth;
        }
        stream->else_emitted[stream->if_depth - 1] = false;
    }
    else if (new_directive->type == PP_DIRECTIVE_ELSE)
    {
        if (!stream->if_depth)
            make_error_directive(lcontext, new_directive->mark, LOG_ERROR, DIRECTIVES_ELSE_OUTSIDE_IF, new_directive);
        else if (stream->else_emitted[stream->if_depth - 1])
            make_error_directive(lcontext, new_directive->mark, LOG_ERROR, DIRECTIVES_ELSE_AFTER_ELSE, new_directive);
        else
            stream->else_emitted[stream->if_depth - 1] = true;
    }
    else if (new_directive->type == PP_DIRECTIVE_ELIF)
    {
        if (!stream->if_depth)
        {
            for (size_t i = 0; i < new_directive->nargs; i++)
                pp_tok_free(new_directive->args[i]);
            make_error_directive(lcontext, new_directive->mark, LOG_ERROR, DIRECTIVES_ELIF_OUTSIDE_IF, new_directive);
        }
        else if (stream->else_emitted[stream->if_depth - 1])
        {
            for (size_t i = 0; i < new_directive->nargs; i++)
                pp_tok_free(new_directive->args[i]);
            make_error_directive(lcontext, new_directive->mark, LOG_ERROR, DIRECTIVES_ELIF_AFTER_ELSE, new_directive);
        }
    }
    else if (new_directive->type == PP_DIRECTIVE_ENDIF)
    {
        if (!stream->if_depth)
            make_error_directive(lcontext, new_directive->mark, LOG_ERROR, DIRECTIVES_ENDIF_WITHOUT_IF, new_directive);
        else
            stream->if_depth--;
    }

    context_free(lcontext);
    return new_directive;
}