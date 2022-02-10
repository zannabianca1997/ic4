/**
 * @file tokenizerio.h
 * @author zannabianca199712 <zannabianca199712@gmail.com>
 * @brief Print tokens to streams
 * @version 0.1
 * @date 2022-02-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _TOKENIZERIO_H
#define _TOKENIZERIO_H

#include <stdio.h>

// --- PRINTING ---

/**
 * @brief Print the token on file
 *
 * @param file the stream to print on
 * @param token the token to print
 * 
 * @return On success, the total number of characters written is returned. If a writing error occurs, the error indicator (ferror) is set and a negative number is returned.
 */
int fprintf_tok(FILE *file, struct pp_token_s const *token);

#ifdef DEBUG
/**
 * @brief Print a human readable representation token on file
 *
 * @param file the stream to print on
 * @param token the token to print
 * 
 * @return On success, the total number of characters written is returned. If a writing error occurs, the error indicator (ferror) is set and a negative number is returned.
 */
int fprintf_repr_tok(FILE *file, struct pp_token_s const *token);
#endif

#endif // _TOKENIZERIO_H