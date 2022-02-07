#define _GNU_SOURCE // needed for fmemopen

#include <stdio.h>
#include <string.h>

#include "lines.h"
#include "linesco.h"

const char *test_rawlines()
{
    char *rawlines = "Row1\nRow2 foo\nRow3 baz\n";
    FILE *rawlines_file = fmemopen(rawlines, strlen(rawlines), "r");


    linestream_t *lstream = linestream_open(NULL, rawlines_file);


    struct logical_line_s *line;

    line = linestream_get(NULL, lstream);
    
    if (line->index[0].row != 1 || line->index[0].start != 0)
        return "First line is misplaced!";
    if (line->index[1].row != 0)
        return "First line is composed of more than one line!";
    if (strcmp(line->content, "Row1"))
        return "First line is inexact";

    return NULL;
}