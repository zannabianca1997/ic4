/**
 * @file directives.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Interface to the directive parser
 * @version 0.1
 * @date 2022-03-06
 *
 * Break a token stream into a stream of directives
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _DIRECTIVES_H
#define _DIRECTIVES_H

#include <stdbool.h>

#include "tokenizer.h"

#include "../misc/bookmark.h"
#include "../misc/context/context.h"

/**
 * @brief Contain a preprocessor directive
 *
 */
struct pp_directive_s
{
    enum pp_directive_type_e
    {
        PP_DIRECTIVE_LINE_CTRL,
        PP_DIRECTIVE_INCLUDE,
        PP_DIRECTIVE_DEFINE,

        PP_DIRECTIVE_IF,
        PP_DIRECTIVE_ELIF,
        PP_DIRECTIVE_ELSE,
        PP_DIRECTIVE_ENDIF,

        PP_DIRECTIVE_ERROR,
        PP_DIRECTIVE_PRAGMA,

        PP_DIRECTIVE_EMIT // special directive, emit the tokens in args
    } type;

    struct bookmark_s mark;

    union
    {
        struct
        {
            bool need_macros;
            union
            {
                struct
                {
                    size_t line_num;
                    char *file_name;
                };
                struct
                {
                    struct pp_token_s **args;
                    size_t nargs;
                };
            };
        } line_ctrl;

        struct
        {
            bool need_macros;
            union
            {
                struct
                {
                    char *file_name;
                    bool is_angled;
                };
                struct
                {
                    struct pp_token_s **args;
                    size_t nargs;
                };
            };
        } include;

        struct
        {
            char *macro_name;
            bool is_function;

            char **args;
            size_t nargs;

            struct pp_token_s **tokens;
            size_t ntokens;
        } define;

        struct
        {
            enum loglevel_e severity;
            char *msg;
        } error;

        struct
        {
            struct pp_token_s **args;
            size_t nargs;
        };
    };
};

/**
 * @brief Delete a directive, freeing the space used
 *
 * @param directive the directive to delete
 */
void directive_free(struct pp_directive_s *directive);

// -- streams --

/**
 * @brief Contain a stream of directives
 *
 */
typedef struct directive_stream_s directive_stream_t;

/**
 * @brief Open a stream of directives
 *
 * @param context the context in which the streams are opened
 * @param source the source of the stream
 * @return directive_stream_t* the opened stream
 */
directive_stream_t *directive_stream_open(context_t *context, pp_tokstream_t *source);
/**
 * @brief Close the stream
 *
 * @param stream the stream to close
 * @param recursive_close if true, the source stream (and it's sources) are closed too
 */
void directive_stream_close(directive_stream_t *stream, bool recursive_close);

/**
 * @brief get a directive from the stream
 *
 * @param context the context in which the directive is needed
 * @param stream the source stream
 * @return struct pp_directive_s* the recovered directive, or NULL if the stream is exausted
 */
struct pp_directive_s *directive_stream_get(context_t *context, directive_stream_t *stream);

#endif // _DIRECTIVES_H