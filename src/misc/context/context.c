#include <stddef.h> // size_t
#include <stdio.h>  // FILE*
#include <stdlib.h> // malloc, free

#include "context.h"
#include "context.cat.h"

#include "../log/log.h"

struct context_s
{
    struct context_s *parent;
#ifdef CHECK_CONTEXT_CHILDS
    size_t child_count;
#endif

    const char *name;
};

context_t *context_new(context_t *parent, const char *context_name)
{
    context_t *new_context = malloc(sizeof(context_t));
    if (new_context == NULL)
        log_error(parent, CONTEXT_FAIL_MALLOC, context_name);

    new_context->parent = parent;
    new_context->name = context_name;

#ifdef CHECK_CONTEXT_CHILDS
    new_context->child_count = 0;
    if (parent != NULL)
        parent->child_count++;
#endif

    return new_context;
}

void context_free(context_t *context)
{
#ifdef CHECK_CONTEXT_CHILDS
    if (context->child_count != 0)
        log_warning(context, CONTEXT_FREE_PARENT);

    if (context->parent != NULL)
        context->parent->child_count--;
#endif
    free(context);
}

void context_fprint(FILE *stream, struct context_s *context)
{
    if (context == NULL)
        return;

    context_fprint(stream, context->parent);
    fprintf(stream, CONTEXT_TRACE, context->name);
}