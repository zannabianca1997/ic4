/* 
    Provide an interface to lines.c
    Break a FILE* stream into logical lines.
    Each line is made of multiple raw lines separated by an escaped newline.
    The raw lines are merged, but where they start is kept
*/
#ifndef _LINES_H
#define _LINES_H

#include <stddef.h> // size_t

#include "../misc/context/context.h"

// --- LOGICAL LINES ---

// contain the data of a single raw line
struct logical_line_s
{
    // the content of the logical line
    char * content;

    // contains the numbering and relative starting position of composing raw lines
    // end marked by a line of num==0
    struct rawline_bookmark_s
    {
        // 1-based line numbering
        size_t row;
        // position of the raw line in the logical one
        size_t start;
    } * index;
};

// free the memory used
// calling with a logical_line not given by linestream_get is UB
void line_free(struct logical_line_s *line);

// --- LOGICAL LINE STREAM ---

// contain all the data for a stream of logical lines
typedef struct linestream_s linestream_t;

// get the next line in a linestream
struct logical_line_s *linestream_get(context_t *context, linestream_t *stream);

#endif // _LINES_H