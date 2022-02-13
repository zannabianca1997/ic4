/**
 * @file test_streamcompare.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Contains methods to create a FILE* that can be checked after against a predefined output
 * @version 0.1
 * @date 2022-02-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _TEST_STREAMCOMPARE_H
#define _TEST_STREAMCOMPARE_H

#include <stdio.h>
#include <stdbool.h>

typedef struct streamcompare_s streamcompare_t;

/**
 * @brief -create a new streamcompare
 * 
 * @param expected the expected output
 * @return streamcompare_t* the streamcompare
 */
streamcompare_t *streamcompare_new(const void *expected, size_t nbytes);
/**
 * @brief free resources from a streamcompare
 * 
 * @param stream the stream to free
 * @return bool result of streamcompare_check
 */
bool streamcompare_free(streamcompare_t *stream);
/**
 * @brief get the FILE to write for checking
 * 
 * @param stream the stream
 * @return FILE* the file
 */
FILE *streamcompare_getfile(streamcompare_t *stream);
/**
 * @brief check if stream is still valid (input was exactly the expected)
 * 
 * @param stream the strea
 * @return true the input up here was right
 * @return false the input differed somewhere
 */
bool streamcompare_check(streamcompare_t *stream);
/**
 * @brief signal if the stream was closed (comparation ended)
 * 
 * @param stream the stream
 * @return true stream is still opened
 * @return false stream was closed, streamcompare_check result is now official
 */
bool streamcompare_open(streamcompare_t *stream);


#endif // _TEST_STREAMCOMPARE_H