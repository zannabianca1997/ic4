/*
    Provide a simple data structure used to keep track of what the program is doing via a tree structure.
*/

#ifndef _CONTEXT_H
#define _CONTEXT_H

#include <stdio.h>

// contain the context in which the program is operating
typedef struct context_s context_t;

// create a child context
// if parent is NULL the context will be root
context_t *context_new(context_t *parent, const char *context_name);

// free a context
void context_free(context_t *context);

// print a context trace on a stream
void context_fprint(FILE *stream, struct context_s *context);

#endif // _CONTEXT_H