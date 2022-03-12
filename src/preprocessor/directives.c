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

#if 0
#pragma GCC warning "<stdio.h> included for debug purposes"
#include <stdio.h>
#endif

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
    queue_t *errors;
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

    return res;
}

void directive_stream_close(directive_stream_t *stream, bool recursive_close)
{
    if (recursive_close)
        pp_tokstream_close(stream->source, true);

    queue_free(stream->errors, &_pp_tok_free_wrapped);
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
static inline struct pp_directive_s *make_error_directive(context_t *lcontext, struct bookmark_s mark, enum loglevel_e level, const char *msg)
{
    struct pp_directive_s *directive = malloc(sizeof(struct pp_directive_s));
    if (directive == NULL)
        log_error(lcontext, DIRECTIVES_MALLOC_FAIL_DIRECTIVE);

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
        if (!queue_push(stream->errors, make_error_directive(context, token->mark, token->error.severity, token->error.msg)))
            log_error(context, DIRECTIVES_QUEUE_ADD_ERROR);

        pp_tok_free(token), token = pp_tokstream_get(context, stream->source);
    }
    return token;
}

static void parse_directive(context_t *context, directive_stream_t *stream, struct pp_directive_s *new_directive)
{
    log_error(context, "Parsing directives is unimplemented"); // TODO
}

static void parse_running_text(context_t *context, directive_stream_t *stream, struct pp_directive_s *new_directive)
{
    queue_t *collected_tokens = queue_new();
    if (collected_tokens == NULL)
        log_error(context, DIRECTIVES_QUEUE_FAIL_CREATING);

    struct pp_token_s *token;
    while ((token = next_token(context, stream)) != NULL && token->type != PP_TOK_DIRECTIVE_START)
    {
        if (!queue_push(collected_tokens, token))
            log_error(context, DIRECTIVES_QUEUE_ADD_FREE_TOKEN);
    }
    pp_tokstream_unget(stream->source, token); // unget the PP_TOK_DIRECTIVE_START

    // creating emit directive
    new_directive->type = PP_DIRECTIVE_EMIT;

    new_directive->nargs = queue_len(collected_tokens);
    new_directive->args = malloc(new_directive->nargs * sizeof(struct pp_token_s *));
    if (new_directive->args == NULL)
        log_error(context, DIRECTIVES_MALLOC_FAIL_EMIT_TOKENS);

    for (size_t idx; !queue_is_empty(collected_tokens); idx++)
        new_directive->args[idx] = queue_pop(collected_tokens);

    new_directive->mark = new_directive->args[0]->mark;
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

    // TODO: add if structure checking

    context_free(lcontext);
    return new_directive;
}