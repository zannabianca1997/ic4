/*
    Provide methods to print a context trace
*/

#ifndef _CONTEXTIO_H
#define _CONTEXTIO_H

#include <stdio.h>

// print a context trace on a stream
void context_fprint(FILE *stream, struct context_s *context);

#endif // _CONTEXTIO_H