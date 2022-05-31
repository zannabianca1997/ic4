/**
 * @file char_stream.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Provide an interface to char_stream.c
 * @version 0.1
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _CHAR_STREAM_H
#define _CHAR_STREAM_H

#include <stddef.h>
#include <stdbool.h>

#include "../misc/bookmark.h"

#define CHAR_UNGET_MAX 1 /**  The maximum number of chars that can be ungetten */
#define _CHAR_ADD_MAX 1  /** The maximum number of char that can be produced (e.g. the newline after \)*/

/**
 * @brief type required for a char source function.
 *
 * The parameter is a magic cookie (e.g. the FILE* object). Must return a negative number on file end.
 */
typedef int source_t(void *cookie);

/**
 * @brief Contain a char and its origin.
 *
 */
struct marked_char
{
    int ch; // Negative if file ended
    struct bookmark mark;
}

/**
 * @brief Contain all data needed for reading a char stream
 *
 */
struct char_stream
{
    // public

    source_t *source; /** The source of the stream */
    void *cookie;     /** the cookie of this source */

    struct marked_char last; /** The last char readed */

    // private

    struct marked_char _unget_buffer[CHAR_UNGET_MAX + _CHAR_ADD_MAX]; /** Buffer to collect ungetted and already processed chars */
    size_t _unget_count;                                              /** How many chars are in the buffer */

    struct bookmark _source_mark; /** The mark of the last char getten from the source */
};

/**
 * @brief Initialize a charstream from the given source
 *
 * @param cs the struct to initialize
 * @param source the char source
 */
void cs_open(struct char_stream *cs, source_t *source);

/**
 * @brief Read a char from a stream
 *
 * @param cs the source stream
 * @return char the char readed
 */
struct marked_char cs_getc(struct char_stream *cs);

/**
 * @brief unget a char to be reread after.
 * Guarantee to work at least CHAR_UNGET_MAX times.
 *
 * @param cs the source stream
 * @param ch the returned char
 * @return true success
 * @return false failure, buffer is full.
 */
bool cs_ungetc(struct char_stream *cs, struct marked_char ch);

#endif // _CHAR_STREAM_H