/* 
    Provide an interface to lines.c opening and closing of linestreams
    Separated from lines.h to protect from the including of large headers like <stdio.h>
*/
#ifndef _LINESCO_H
#define _LINESCO_H

#include <stdio.h> // FILE*
#include <stdbool.h> // bool

#include "lines.h"
#include "../misc/context/context.h"

// --- LOGICAL LINE STREAM ---

// open a linestream from file
linestream_t *linestream_open(context_t *context, FILE *source);

// close a linestream
// if recursive_close the underling stream is closed too
void linestream_close(linestream_t *stream, bool recursive_close);

//TODO: write linestream setup struct (e.g. if warn of \ as last non-ws char, or last line is non-empty)

#endif // _LINESCO_H