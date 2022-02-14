/**
 * @file tokenizerco.h
 * @author zannabianca199712 <zannabianca199712@gmail.com>
 * @brief Open and close a stream of preprocessing tokens
 * @version 0.1
 * @date 2022-02-10
 * 
 * Interface to the closing and opening functionalities of tokenizer.c
 * This isolate the other modules from including lines.h
 * 
 * @copyright Copyright (c) 2022
 */

#ifndef _TOKENIZERCO_H
#define _TOKENIZERCO_H

#include <stdbool.h>

#include "tokenizer.h"
#include "lines.h"

/**
 * @brief Open a new token stream from a linestream
 * 
 * Open a new token stream from a linestream.
 * New token stream starts from the next logical line in stream
 * 
 * @param context the context for which the token stream is needed
 * @param source the source of the lines
 * @return pp_tokstream_t* the opened stream
 */
pp_tokstream_t *pp_tokstream_open(context_t *context, linestream_t *source);

#endif // _TOKENIZERCO_H