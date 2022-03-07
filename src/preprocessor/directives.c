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

#include "directives.h"

#include "../misc/log/log.h"

#include "messages.cat.h"

// -- directive end of life

void directive_free(struct pp_directive_s *directive)
{
    switch (directive->type)
    {
    case PP_DIRECTIVE_LINE_CTRL:
        if (directive->line_ctrl.need_macros)
            queue_free(directive->line_ctrl.args, &pp_tok_free);
        else
            free(directive->line_ctrl.file_name);
        break;

    case PP_DIRECTIVE_INCLUDE:
        if (directive->include.need_macros)
            queue_free(directive->include.args, &pp_tok_free);
        else
            free(directive->include.file_name);
        break;

    case PP_DIRECTIVE_DEFINE:
        pp_tok_free(directive->define.macro_name);
        for (size_t i = 0; i < directive->define.nargs; i++)
            free(directive->define.args[i]);
        free(directive->define.args);
        queue_free(directive->define.definition, &pp_tok_free);
        break;

    case PP_DIRECTIVE_IF:
    case PP_DIRECTIVE_ELIF:
        queue_free(directive->condition, &pp_tok_free);
        break;

    case PP_DIRECTIVE_ERROR:
    case PP_DIRECTIVE_PRAGMA:
    case PP_DIRECTIVE_EMIT:
        queue_free(directive->args, &pp_tok_free);
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


