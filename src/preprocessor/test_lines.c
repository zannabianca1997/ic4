#define _GNU_SOURCE // needed for fmemopen

#include <stdio.h>
#include <string.h>

#include "lines.h"
#include "linesco.h"

const char *test_rawlines()
{
    //TODO: write this mess in a more streamlined way
    //      maybe referencing a catalog in some way
    char *rawlines = "Row1\nRow2 foo\nRow3 baz \n";
    FILE *rawlines_file = fmemopen(rawlines, strlen(rawlines), "r");

    linestream_t *lstream = linestream_open(NULL, rawlines_file);

    struct logical_line_s *line;

    line = linestream_get(NULL, lstream);

    if (line == NULL)
        return "First line wasn't produced";
    if (line->index[0].row != 1 || line->index[0].start != 0)
        return "First line is misplaced!";
    if (line->index[1].row != 0)
        return "First line is composed of more than one line!";
    if (strcmp(line->content, "Row1"))
        return "First line is inexact";

    line = linestream_get(NULL, lstream);

    if (line == NULL)
        return "Second line wasn't produced";
    if (line->index[0].row != 2 || line->index[0].start != 0)
        return "Second line is misplaced!";
    if (line->index[1].row != 0)
        return "Second line is composed of more than one line!";
    if (strcmp(line->content, "Row2 foo"))
        return "Second line is inexact";

    line = linestream_get(NULL, lstream);

    if (line == NULL)
        return "Third line wasn't produced";
    if (line->index[0].row != 3 || line->index[0].start != 0)
        return "Third line is misplaced!";
    if (line->index[1].row != 0)
        return "Third line is composed of more than one line!";
    if (strcmp(line->content, "Row3 baz "))
        return "Third line is inexact";

    line = linestream_get(NULL, lstream);

    if (line == NULL)
        return "Fourth (empty) line wasn't produced";
    if (line->index[0].row != 4 || line->index[0].start != 0)
        return "Fourth (empty) line is misplaced!";
    if (line->index[1].row != 0)
        return "Fourth (empty) line is composed of more than one line!";
    if (strcmp(line->content, ""))
        return "Fourth (empty) line is inexact";

    line = linestream_get(NULL, lstream);

    if (line != NULL)
        return "A fifth line (not in the input) was produced!";

    return NULL;
}