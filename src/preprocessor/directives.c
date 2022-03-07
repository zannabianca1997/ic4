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

#include "directives.h"

#include "../misc/queue.h"
#include "../misc/log/log.h"

#include "messages.cat.h"

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

    case PP_DIRECTIVE_IF:
    case PP_DIRECTIVE_ELIF:
    case PP_DIRECTIVE_ERROR:
    case PP_DIRECTIVE_PRAGMA:
    case PP_DIRECTIVE_EMIT:
        for (size_t i = 0; i < directive->nargs; i++)
            pp_tok_free(directive->args[i]);
        free(directive->args);
        break;

    case PP_DIRECTIVE_ELSE:
    case PP_DIRECTIVE_ENDIF:
        break;
    }

    free(directive);
}

// -- directive stream

/**
 * @brief Contain a stream of directives
 *
 */
struct directive_stream_s
{
    pp_tokstream_t *source;
};

directive_stream_t *directive_stream_open(context_t *context, pp_tokstream_t *source)
{
    directive_stream_t *res = malloc(sizeof(directive_stream_t));
    if (res == NULL)
        log_error(context_new(context, DIRECTIVES_CONTEXT_OPENING), DIRECTIVES_MALLOC_FAIL_OPEN);

    res->source = source;

    return res;
}

void directive_stream_close(directive_stream_t *stream, bool recursive_close)
{
    if (recursive_close)
        pp_tokstream_close(stream->source, true);
    free(stream);
}

// -- parsing

/**
 * @brief Create an error directive
 *
 * @param lcontext the context of the directive creation
 * @param directive the directive to initialize
 * @param mark the position of the error
 * @param level the severity of the error
 * @param msg the message of the error
 */
static inline void make_error_directive(context_t *lcontext, struct pp_directive_s *directive, struct bookmark_s mark, enum loglevel_e level, const char *msg)
{
    directive->type = PP_DIRECTIVE_ERROR;
    directive->mark = mark;
    directive->error.severity = level;

    directive->error.msg = malloc(strlen(msg) + 1);
    if (directive->error.msg == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_STRDUP);
    strcpy(directive->error.msg, msg);
}

static inline void parse_macro_args(context_t *lcontext, struct pp_directive_s *directive, directive_stream_t *stream)
{
    struct pp_token_s *token = pp_tokstream_get(lcontext, stream->source);
    if (token->type == PP_TOK_PUNCTUATOR && token->punc_kind == PUNC_PAR_RIGHT)
    {
        directive->define.nargs = 0;
        return;
    }

    queue_t *args;
    if ((args = queue_new()) == NULL)
        log_error(lcontext, DIRECTIVES_QUEUE_FAIL_CREATING);

    while (token->type == PP_TOK_IDENTIFIER)
    {
        char *name = malloc(strlen(token->name) + 1);
        if (name == NULL)
            log_error(lcontext, DIRECTIVES_MALLOC_FAIL_STRDUP);
        strcpy(name, token->name);

        if (!queue_push(args, name))
            log_error(lcontext, DIRECTIVES_QUEUE_ADD_ARG);

        pp_tok_free(token), token = pp_tokstream_get(lcontext, stream->source);
        if (token->type != PP_TOK_PUNCTUATOR || !(token->punc_kind == PUNC_PAR_RIGHT || token->punc_kind == PUNC_COMMA))
        {
            queue_free(args, &free);
            make_error_directive(lcontext, directive, token->mark, LOG_ERROR, DIRECTIVES_ERROR_COMMA_OR_LPAR_EXPECTED);
            return;
        }

        if (token->punc_kind == PUNC_PAR_RIGHT)
        {
            directive->define.nargs = queue_len(args);
            directive->define.args = malloc(directive->define.nargs * sizeof(char *));
            if (directive->define.args == NULL)
                log_error(lcontext, DIRECTIVES_MALLOC_FAIL_DEFINE_ARGS);

            for (char **ptr = directive->define.args; !queue_is_empty(args); ptr++)
                *ptr = queue_pop(args);
            queue_free(args, NULL);

            return;
        }

        pp_tok_free(token), token = pp_tokstream_get(lcontext, stream->source);
    }

    pp_tok_free(token);
    queue_free(args, &free);
    make_error_directive(lcontext, directive, token->mark, LOG_ERROR, DIRECTIVES_ERROR_IDENTIFIER_EXPECTED);
}

static inline void parse_define(context_t *lcontext, struct pp_directive_s *directive, directive_stream_t *stream)
{
    struct pp_token_s *token = pp_tokstream_get(lcontext, stream->source);
    if (token->type != PP_TOK_MACRO_NAME)
    {
        make_error_directive(lcontext, directive, token->mark, LOG_ERROR, DIRECTIVES_ERROR_MACRO_NAME);
        // clean rest of directive
        while (token->type != PP_TOK_DIRECTIVE_STOP)
            pp_tok_free(token), token = pp_tokstream_get(lcontext, stream->source);
        pp_tok_free(token);
    }

    // copy macro name
    directive->define.macro_name = malloc(strlen(token->macro_name.name) + 1);
    if (directive->define.macro_name == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_STRDUP);
    strcpy(directive->define.macro_name, token->macro_name.name);
    directive->define.is_function = token->macro_name.is_function;

    // reading params
    if (directive->define.is_function)
        parse_macro_args(lcontext, directive, stream);

    // reading definition
    queue_t *tokens;
    if ((tokens = queue_new()) == NULL)
        log_error(lcontext, DIRECTIVES_QUEUE_FAIL_CREATING);
    pp_tok_free(token), token = pp_tokstream_get(lcontext, stream->source);
    while (token->type != PP_TOK_DIRECTIVE_STOP)
    {
        queue_push(tokens, token);
        token = pp_tokstream_get(lcontext, stream->source);
    }
    pp_tok_free(token);

    // making traditional array
    directive->define.ntokens = queue_len(tokens);
    directive->define.tokens = malloc(directive->define.ntokens * sizeof(struct pp_token_s *));
    if (directive->define.tokens == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_DEFINE_TOKENS);

    for (struct pp_token_s **ptr = directive->define.tokens; !queue_is_empty(tokens); ptr++)
        *ptr = queue_pop(tokens);
    queue_free(tokens, NULL);
}

struct pp_directive_s *directive_stream_get(context_t *context, directive_stream_t *stream)
{
    context_t *lcontext = context_new(context, DIRECTIVES_CONTEXT_GETTING);
    struct pp_token_s *token = pp_tokstream_get(lcontext, stream->source);
    if (token == NULL)
    {
        context_free(lcontext);
        return NULL;
    }

    struct pp_directive_s *new_directive = malloc(sizeof(struct pp_directive_s));
    if (new_directive == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_DIRECTIVE);

    if (token->type != PP_TOK_DIRECTIVE_START)
    {
        new_directive->type = PP_DIRECTIVE_EMIT;
        new_directive->mark = token->mark;

        // collecting free text tokens
        queue_t *tokens;
        if ((tokens = queue_new()) == NULL)
            log_error(lcontext, DIRECTIVES_QUEUE_FAIL_CREATING);
        do
        {
            if (!queue_push(tokens, token))
                log_error(lcontext, DIRECTIVES_QUEUE_ADD_FREE_TOKEN);
            token = pp_tokstream_get(lcontext, stream->source);
        } while (token != NULL && token->type != PP_TOK_DIRECTIVE_START);
        pp_tokstream_unget(stream->source, token); // give back last token

        new_directive->nargs = queue_len(tokens);
        new_directive->args = malloc(new_directive->nargs * sizeof(struct pp_token_s *));
        if (new_directive->args == NULL)
            log_error(lcontext, DIRECTIVES_MALLOC_FAIL_DEFINE_TOKENS);

        for (struct pp_token_s **ptr = new_directive->args; !queue_is_empty(tokens); ptr++)
            *ptr = queue_pop(tokens);
        queue_free(tokens, NULL);
    }
    else
    {
        pp_tok_free(token), token = pp_tokstream_get(lcontext, stream->source);
        if (token->type != PP_TOK_IDENTIFIER)
        {
            make_error_directive(lcontext, new_directive, token->mark, LOG_ERROR, DIRECTIVES_ERROR_NAME);
            // clean rest of directive
            while (token->type != PP_TOK_DIRECTIVE_STOP)
                pp_tok_free(token), token = pp_tokstream_get(lcontext, stream->source);
            pp_tok_free(token);
        }
        else
        {
            if (strcmp(token->name, "define") == 0)
            {
                new_directive->type = PP_DIRECTIVE_DEFINE;
                new_directive->mark = token->mark;
                parse_define(lcontext, new_directive, stream);
            }
            else if (strcmp(token->name, "include") == 0)
            {
                log_error(lcontext, "Unimplemented directive");
            }
            else if (strcmp(token->name, "line") == 0)
            {
                log_error(lcontext, "Unimplemented directive");
            }
            else if (strcmp(token->name, "if") == 0)
            {
                log_error(lcontext, "Unimplemented directive");
            }
            else if (strcmp(token->name, "elif") == 0)
            {
                log_error(lcontext, "Unimplemented directive");
            }
            else if (strcmp(token->name, "else") == 0)
            {
                log_error(lcontext, "Unimplemented directive");
            }
            else if (strcmp(token->name, "endif") == 0)
            {
                log_error(lcontext, "Unimplemented directive");
            }
            else if (strcmp(token->name, "error") == 0)
            {
                log_error(lcontext, "Unimplemented directive");
            }
            else if (strcmp(token->name, "pragma") == 0)
            {
                log_error(lcontext, "Unimplemented directive");
            }
            else
            {
                make_error_directive(lcontext, new_directive, token->mark, LOG_ERROR, DIRECTIVES_ERROR_UNKNOW);
                // clean rest of directive
                while (token->type != PP_TOK_DIRECTIVE_STOP)
                    pp_tok_free(token), token = pp_tokstream_get(lcontext, stream->source);
                pp_tok_free(token);
            }
        }
    }

    context_free(lcontext);
    return new_directive;
}