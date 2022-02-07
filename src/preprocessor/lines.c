/*
    Split a FILE* into logical line (escape newline)
*/

#include <stdio.h>  // FILE*, getc, ungetc
#include <stdlib.h> // malloc, realloc, free
#include <stddef.h> // size_t

#include "../misc/log/log.h"

#include "lines.h"
#include "linesco.h"

#include "messages.cat.h"

static const char ESCAPE_CHAR = '/';

#ifndef RAWLINE_BUFFER_INITIAL_LEN
static const size_t RAWLINE_BUFFER_INITIAL_LEN = 256;
#endif

#ifndef RAWLINE_BUFFER_GROWRATE
static const size_t RAWLINE_BUFFER_GROWRATE = 2;
#endif

// --- LOGICAL LINE ---

void line_free(struct logical_line_s *line)
{
    free(line->content);
    free(line->index);
    free(line);
}

// --- LOGICAL LINE STREAM ---

struct linestream_s
{
    FILE *source;
    size_t rawline_readed;
};

linestream_t *linestream_open(context_t *context, FILE *source)
{
    linestream_t *new_stream = malloc(sizeof(linestream_t));
    if (new_stream == NULL)
        log_error(context, LINESTREAM_MALLOC_FAIL_OPEN);

    new_stream->source = source;
    new_stream->rawline_readed = 0;

    return new_stream;
}
void linestream_close(linestream_t *stream, bool close_file)
{
    if (close_file)
        fclose(stream->source);
    free(stream);
}

// if stream begin with a newline, consume it and return True
// else return False and leave stream invariated
static bool linestream_peek_newline(FILE *stream)
{
    // get next char
    int ch = getc(stream);
    // is a Unix-style newline?
    if (ch == '\n')
        return true;
    // is a Windows or MacOS?
    if (ch == '\r')
    {
        // get the next one
        ch = getc(stream);
        // if it's not Windows, put it back
        if (ch != '\n' && ch != EOF)
            ungetc(ch, stream);
        return true;
    }
    // else, return the char to the stream
    if (ch != EOF)
        ungetc(ch, stream);
    return false;
}

struct logical_line_s *linestream_get(context_t *context, linestream_t *stream)
{
    // short circuit if file is ended
    if (feof(stream->source))
        return NULL;
        
    // creating local context
    context_t *lcontext = context_new(context, LINESTREAM_CONTEXT_READING);

    // opening space for return struct
    struct logical_line_s *new_logical_line = malloc(sizeof(struct logical_line_s));
    if (new_logical_line == NULL)
        log_error(lcontext, LINESTREAM_MALLOC_FAIL_LOGICALLINE);


    // allocating space for line buffer
    size_t buffer_len = RAWLINE_BUFFER_INITIAL_LEN;
    new_logical_line->content = malloc(sizeof(char) * buffer_len);
    if (new_logical_line->content == NULL)
        log_error(lcontext, LINESTREAM_MALLOC_FAIL_EXTENDBUFFER);

    // the index is kept so it contain a line MORE than the one actually readed
    // this avoid the last realloc to put the ending entry
    new_logical_line->index = malloc(sizeof(struct rawline_bookmark_s) * 2);
    if (new_logical_line->index == NULL)
        log_error(lcontext, LINESTREAM_MALLOC_FAIL_EXTENDLINES);
    new_logical_line->index->row = stream->rawline_readed + 1;
    new_logical_line->index->start = 0;

    size_t char_readed = 0;
    size_t rawline_num = 0;

    while (!linestream_peek_newline(stream->source))
    {
        int ch = getc(stream->source);
        // checking if we need to escape a newline
        if (ch == ESCAPE_CHAR && linestream_peek_newline(stream->source))
        {
            // escaped newline -> new rawline

            // growing the index one by one, we do not expect a lot of those
            rawline_num++;

            // growed buffer
            struct rawline_bookmark_s *new_index = realloc(new_logical_line->index, sizeof(struct rawline_bookmark_s) * (rawline_num + 2));
            if (new_index == NULL)
                log_error(lcontext, LINESTREAM_MALLOC_FAIL_EXTENDLINES);
            new_logical_line->index = new_index;

            // setting up new entry
            new_logical_line->index[rawline_num].row = stream->rawline_readed + rawline_num + 1;
            new_logical_line->index[rawline_num].start = char_readed;

            continue; // do not add ch, go to the next char

        }
        // check for file ended
        if (ch == EOF)
            break; // this silently consider EOF a newline
        // adding ch
        new_logical_line->content[char_readed++] = ch;
        if (char_readed >= buffer_len)
        {
            // need to extend the buffer, for next char or \0
            buffer_len *= RAWLINE_BUFFER_GROWRATE;
            char *newbuffer = realloc(new_logical_line->content, sizeof(char) * buffer_len);
            if (newbuffer == NULL)
                log_error(lcontext, LINESTREAM_MALLOC_FAIL_EXTENDBUFFER);
            new_logical_line->content = newbuffer;
        }
    }


    // putting endings
    new_logical_line->index[rawline_num + 1].row = 0;
    new_logical_line->content[char_readed] = '\0';

#ifdef LOGICALLINE_SHRINK
    // shrinking buffers to fit
    char *newbuffer = realloc(new_logical_line->content, sizeof(char) * (char_readed + 1));
    if (newbuffer == NULL)
        log_error(lcontext, LINESTREAM_MALLOC_FAIL_SHRINKBUFFER);
    new_logical_line->content = newbuffer;

    // index do not need to be shrink, given it grows one by one
#endif

    // bookkeeping
    stream->rawline_readed += rawline_num + 1;

    context_free(lcontext);

    return new_logical_line;
}