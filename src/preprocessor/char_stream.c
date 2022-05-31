/**
 * @file char_stream.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Read a list of char from a stream, performing basic charset transformation,
 *        line counting and trigraph elimination
 * @version 0.1
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "char_stream.h"

void cs_open(struct char_stream *cs, source_t *source)
{
    cs->source = source;

    cs->_unget_count = 0;
    cs->_source_mark = (struct bookmark){1, 0, NULL};
}

/**
 * @brief get the next char to process, either from the buffer or from the stream
 *
 */
static struct marked_char cs_next_char(struct char_stream *cs)
{
    if (cs->_unget_count != 0)
        return cs->_unget_buffer[--(cs->_unget_count)];
    else
    {
        // getting a new char
        char ch = (*(cs->source))(cs->cookie);
        bm_count(&(cs->_source_mark), ch);
        return (struct marked_char){ch, cs->_source_mark};
    }
}

struct marked_char cs_getc(struct char_stream *cs)
{
    struct marked_char ch = cs_next_char(cs);

    // escaped newlines
    while (ch.ch == '\\')
    {
        struct marked_char escaped = cs_next_char(cs);
        if (escaped.ch != '\n')
        {
            cs_ungetc(cs, escaped); // put the escaped char back
            break;
        }
        ch = cs_next_char(cs); // ignore both this newline and the next char
    }

    return (cs->last = ch);
}

bool cs_ungetc(struct char_stream *cs, struct marked_char ch)
{
#ifdef CHECK_UNGETC
    if (_unget_count == CHAR_UNGET_MAX + _CHAR_ADD_MAX)
        return false;
#endif
    cs->_unget_buffer[(cs->_unget_count)++] = ch;
    return true;
}