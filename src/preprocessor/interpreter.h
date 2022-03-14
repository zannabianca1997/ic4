/**
 * @file interpreter.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Describe an interface to the preprocessor language
 * @version 0.1
 * @date 2022-03-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "tokenizer.h"

#include "../misc/bookmark.h"
#include "../misc/context/context.h"

/**
 * @brief Contain the interpreter for the preprocessor language
 *
 */
typedef struct pp_interpreter_s pp_interpreter_t;

/**
 * @brief get the next token from the output of the preprocessor
 *
 * @param context the context in wich the token is required
 * @param interpreter the interpreter giving the token
 * @return struct pp_token_s* the token given, or NULL for end of input
 */
struct pp_token_s *pp_interpreter_get(context_t *context, pp_interpreter_t *interpreter);

/**
 * @brief Close the interpreter
 *
 * @param interpreter the interpreter to close
 * @param recursive_close if the underlying sources need to be closed
 */
void pp_interpreter_close(pp_interpreter_t *interpreter, bool recursive_close);

#endif // _INTERPRETER_H