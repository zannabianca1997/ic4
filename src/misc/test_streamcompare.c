/**
 * @file test_streamcompare.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Contains methods to create a FILE* that can be checked after against a predefined output
 * @version 0.1
 * @date 2022-02-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <memory.h>
#include <stdlib.h>

#include "test_streamconpare.h"

/**
 * @brief All data regarding a streamcompare
 * 
 */
struct streamcompare_s
{
    /**
     * @brief the expected data
     * 
     */
    void *expected;
    /**
     * @brief the expected data remaining
     * 
     */
    void const *expected_remain;
    /**
     * @brief len of unmatched data
     */
    size_t remain_len;

    /**
     * @brief the input stream
     * 
     */
    FILE *in_stream;
    /**
     * @brief true if all data up until there matched true
     * 
     */
    bool check;
    /**
     * @brief true if file is strill open
     * 
     */
    bool open;
};

// -- stream function

static ssize_t write(void *cookie, const void *buf, size_t nbytes)
{
    struct streamcompare_s *stream = cookie;

    // fail if fclose was called
    if (!stream->open)
        return 0;

    // success if already failed
    if (!stream->check)
        return nbytes;

    // success and mark comparison as failed if too long
    if (stream->remain_len < nbytes)
    {
        stream->check = false;
        return nbytes;
    }

    // compare the memory
    if (memcmp(buf, stream->expected_remain, nbytes) != 0)
    {
        // comparation failed
        stream->check = false;
        // mark write as success
        return nbytes;
    }
    else
    {
        // stream is still valid
        stream->expected_remain = (void const *)(((char const *)(stream->expected_remain)) + nbytes);
        stream->remain_len -= nbytes;
        // mark write as success
        return nbytes;
    }
}
static int close(void *cookie)
{
    struct streamcompare_s *stream = cookie;

    // if there is still data to match, comparison fails
    if (stream->remain_len > 0)
        stream->check = false;

    // free expected buffer
    free(stream->expected);
    stream->expected = stream->expected_remain = NULL;
    stream->remain_len = 0;

    // mark stream as closed
    stream->open = false;

    return 0;
}

streamcompare_t *streamcompare_new(const void *expected, size_t nbytes)
{
    // allocate memory
    streamcompare_t *new_stream = malloc(sizeof(streamcompare_t));
    if (new_stream == NULL)
        return NULL;
    void *new_expected = malloc(nbytes);
    if (new_expected == NULL)
    {
        free(new_stream);
        return NULL;
    }

    // copy expected results
    memcpy(new_expected, expected, nbytes);

    // set up stream

    // buffers for expected data
    new_stream->expected = new_expected;
    new_stream->expected_remain = new_stream->expected;
    new_stream->remain_len = nbytes;

    // flags
    new_stream->check = true;
    new_stream->open = true;

    // open FILE
    new_stream->in_stream = fopencookie(new_stream, "w", (cookie_io_functions_t){
        .read = NULL,
        .write = &write,
        .seek = NULL,
        .close = &close
    });
    if(new_stream->in_stream == NULL){
        free(new_stream->expected);
        free(new_stream);
        return NULL;
    }

    return new_stream;
}
bool streamcompare_free(streamcompare_t *stream){
    // flush and close stream
    fclose(stream->in_stream);

    // get compare result
    bool last_check = streamcompare_check(stream);

    // free the memory
    free(stream);

    return last_check;
}
FILE *streamcompare_getfile(streamcompare_t *stream){
    return stream->in_stream;
}
bool streamcompare_check(streamcompare_t *stream){
    return stream->check;
}
bool streamcompare_open(streamcompare_t *stream){
    return stream->open;
}