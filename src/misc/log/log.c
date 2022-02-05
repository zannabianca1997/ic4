/*
    Manage multiple logging targets: 
    what level of logging is possible in any of them and other characteristic (color)
    Close the application when a error is logged
*/

#include <stdbool.h> // bool, true, false
#include <stddef.h>  // ptrdiff_t, size_t
#include <stdio.h>   // FILE*, printf family
#include <stdlib.h>  // malloc, realloc, free, exit
#include <stdarg.h>  // variadic argument support

#include "../context/context.h"

#include "loglevel.h"
#include "log.h"
#include "logtarget.h"

#include "log.cat.h"

// --- ABORTLEVEL ---

static enum loglevel_e abortlevel = LOG_ERROR;
static void (*abortfun)(context_t *) = NULL;

void set_abortlevel(context_t *context, enum loglevel_e new_abortlevel)
{
#ifdef DEBUG
    // checking validity of the settings
    if (new_abortlevel > LOG_ERROR)
        log_error(context_new(context, ABORTLEVEL_SETTING_ABORTLEVEL), ABORTLEVEL_ABORTLEVEL_OVER_LOG_ERROR, new_abortlevel, LOG_ERROR);
#endif

    abortlevel = new_abortlevel;
}

void set_abortfunc(void (*new_abortfun)(context_t *))
{
    abortfun = new_abortfun;
}

// --- MANAGING TARGETS ---

// contain all the data about a target
struct logtarget_s
{
    FILE *target;
    struct logtarget_errorlevels_s errorlevels;

#ifdef ANSI_FORMATTED_OUTPUT
    bool use_ansi_format;
    struct logtarget_ansi_format_s format;
#endif
};

// array of all the managed logtargets
static logtarget_t **logtargets = NULL;
// how many targets are there
static size_t logtargets_num = 0;
// maximum number of target available
static size_t logtargets_max = 0;

logtarget_t *logtarget_new(context_t *context, FILE *target, struct logtarget_errorlevels_s errorlevels)
{
    if (logtargets_num == logtargets_max)
    {
        // it's not expected to add a lot of target, so we open space one by one
        logtarget_t **new_logtargets = realloc(logtargets, sizeof(logtarget_t *) * (logtargets_max + 1));
        if (new_logtargets == NULL)
            log_error(context_new(context, LOGTARGET_CONTEXT_GENERATING), LOGTARGET_FAIL_MALLOC_PTR);

        logtargets = new_logtargets;
        logtargets_max++;
    }

    // allocating space for the new target
    logtargets[logtargets_num] = malloc(sizeof(logtarget_t));
    if (logtargets[logtargets_num] == NULL)
        log_error(context_new(context, LOGTARGET_CONTEXT_GENERATING), LOGTARGET_FAIL_MALLOC);

    // setting up new target
    (*logtargets[logtargets_num]).target = target;
    (*logtargets[logtargets_num]).errorlevels = errorlevels;

#ifdef ANSI_FORMATTED_OUTPUT
    (*logtargets[logtargets_num]).use_ansi_format = false;
#endif

    // marking new target as disponible
    logtargets_num++;

    return logtargets[logtargets_num - 1];
}

void logtarget_remove(context_t *context, logtarget_t *target, bool close_stream)
{
    if (close_stream)
        fclose(target->target);

    // searching for target in the managed targets
    size_t idx = 0;
    while (logtargets[idx] != target)
    {
        idx++;
#ifdef DEBUG
        if (idx >= logtargets_num)
            log_error(context_new(context, LOGTARGET_CONTEXT_FREEING), LOGTARGET_TARGET_PTR_UNKNOW);
#endif
    }

    // moving all managed targets down to close the gap
    for (; idx + 1 < logtargets_num; idx++)
        logtargets[idx] = logtargets[idx + 1];

    // updating logtarget num
    logtargets_num--;

    // we don't touch logtarget_max. Probably logtarget_num wont fluctuate much

    // freeing logtarget
    free(target);
}

// --- SETTING ERRORLEVELS ---

void logtarget_set_errorlevels(context_t *context, logtarget_t *target, struct logtarget_errorlevels_s errorlevels)
{
#ifdef DEBUG
    // checking validity of the settings
    if (errorlevels.debuglevel > errorlevels.warninglevel)
        log_error(context_new(context, LOGLEVEL_SETTING_LOGLEVELS), LOGLEVEL_DEBUGLEVEL_OVER_WARNINGLEVEL, errorlevels.debuglevel, errorlevels.warninglevel);
    if (errorlevels.warninglevel > errorlevels.errorlevel)
        log_error(context_new(context, LOGLEVEL_SETTING_LOGLEVELS), LOGLEVEL_WARNINGLEVEL_OVER_ERRORLEVEL, errorlevels.warninglevel, errorlevels.errorlevel);
    if (errorlevels.errorlevel > LOG_ERROR)
        log_error(context_new(context, LOGLEVEL_SETTING_LOGLEVELS), LOGLEVEL_ERRORLEVEL_OVER_LOG_ERROR, errorlevels.errorlevel, LOG_ERROR);
#endif

    target->errorlevels = errorlevels;
}

void logtarget_set_debuglevel(context_t *context, logtarget_t *target, enum loglevel_e new_debuglevel)
{
#ifdef DEBUG
    // checking validity of the settings
    if (new_debuglevel > target->errorlevels.warninglevel)
        log_error(context_new(context, LOGLEVEL_SETTING_DEBUGLEVEL), LOGLEVEL_DEBUGLEVEL_OVER_WARNINGLEVEL, new_debuglevel, target->errorlevels.warninglevel);
#endif

    target->errorlevels.debuglevel = new_debuglevel;
}

void logtarget_set_warninglevel(context_t *context, logtarget_t *target, enum loglevel_e new_warninglevel)
{
#ifdef DEBUG
    // checking validity of the settings
    if (new_warninglevel > target->errorlevels.errorlevel)
        log_error(context_new(context, LOGLEVEL_SETTING_WARNINGLEVEL), LOGLEVEL_WARNINGLEVEL_OVER_ERRORLEVEL, new_warninglevel, target->errorlevels.errorlevel);
#endif

    target->errorlevels.warninglevel = new_warninglevel;
}

void logtarget_set_errorlevel(context_t *context, logtarget_t *target, enum loglevel_e new_errorlevel)
{
#ifdef DEBUG
    // checking validity of the settings
    if (new_errorlevel > LOG_ERROR)
        log_error(context_new(context, LOGLEVEL_SETTING_ERRORLEVEL), LOGLEVEL_ERRORLEVEL_OVER_LOG_ERROR, new_errorlevel, LOG_ERROR);
#endif

    target->errorlevels.errorlevel = new_errorlevel;
}

// --- LOGGING DISPATCHING AND PRINTING ---

// print log message to the target
static void logtarget_print(logtarget_t *target, context_t *context, enum loglevel_e level, struct bookmark_s bookmark, const char *fmt, va_list vlist)
{
    // check if we have to print it at all
    if (level < target->errorlevels.debuglevel)
        return;

    const char *mextype;
    if (level < target->errorlevels.warninglevel)
        mextype = LOGLEVEL_DEBUG;
    else if (level < target->errorlevels.errorlevel)
        mextype = LOGLEVEL_WARNING;
    else
        mextype = LOGLEVEL_ERROR;

    // we trace the context
    context_fprint(target->target, context);
    // if needed, we print the bookmark
    if (bookmark.filename != NULL)
    {
        if (bookmark.row)
        {
            if (bookmark.col)
                fprintf(target->target, LOGFORMAT_BOOKMARK_COMPLETE, bookmark.filename, bookmark.row, bookmark.col);
            else
                fprintf(target->target, LOGFORMAT_BOOKMARK_NOCOL, bookmark.filename, bookmark.row);
        }
        else
            // col is ignored if row is missing
            fprintf(target->target, LOGFORMAT_BOOKMARK_ONLYFILE, bookmark.filename);

        fprintf(target->target, LOGFORMAT_SEPARATOR);
    }
    else if (bookmark.row)
    {
        if (bookmark.col)
            fprintf(target->target, LOGFORMAT_BOOKMARK_NOFILE, bookmark.row, bookmark.col);
        else
            fprintf(target->target, LOGFORMAT_BOOKMARK_ONLYLINE, bookmark.row);

        fprintf(target->target, LOGFORMAT_SEPARATOR);
    }

    // print the level
    fprintf(target->target, "%s", mextype);
    fprintf(target->target, LOGFORMAT_SEPARATOR);
    // print the message
    vfprintf(target->target, fmt, vlist);
    // add the message separator
    fprintf(target->target, LOGFORMAT_MESSAGE_SEPARATOR);
}

// dispatch message to all the targets
static void logtarget_dispatch(context_t *context, enum loglevel_e level, struct bookmark_s bookmark, const char *fmt, va_list vlist)
{
    // log to all targets
    for (size_t i = 0; i < logtargets_num; i++)
    {
        va_list vlist_copy;
        va_copy(vlist_copy, vlist);
        logtarget_print(logtargets[i], context, level, bookmark, fmt, vlist_copy);
        va_end(vlist_copy);
    }

    // check if abort, and in case execute it
    if (level >= abortlevel)
    {
        if (abortfun != NULL)
            (*abortfun)(context);
        exit(1);
    }

#ifdef __GNUC__
    // we signal to gcc that if level>=LOG_ERROR the function won't return
    if (level >= LOG_ERROR)
        __builtin_unreachable();
#endif
}

// --- LOG FUNCTIONS ---

void log_log(context_t *context, enum loglevel_e level, const char *fmt, ...)
{
    va_list vlist;
    va_start(vlist, fmt);
    logtarget_dispatch(context, level, (struct bookmark_s){NULL, 0, 0}, fmt, vlist);
    va_end(vlist);
}

// generating all the specialized one from the same definition
#define SPECIALIZED_LOG_F(name, NAME, post)                                             \
    void log_##name(context_t *context, const char *fmt, ...)                           \
    {                                                                                   \
        va_list vlist;                                                                  \
        va_start(vlist, fmt);                                                           \
        logtarget_dispatch(context, NAME, (struct bookmark_s){NULL, 0, 0}, fmt, vlist); \
        va_end(vlist);                                                                  \
        post;                                                                           \
    }

SPECIALIZED_LOG_F(trace, LOG_TRACE, )
SPECIALIZED_LOG_F(debug, LOG_DEBUG, )
SPECIALIZED_LOG_F(pedantic, LOG_PEDANTIC, )
SPECIALIZED_LOG_F(warning, LOG_WARNING, )

#ifdef __GNUC__
__attribute__((noreturn)) SPECIALIZED_LOG_F(error, LOG_ERROR, __builtin_unreachable())
#else
SPECIALIZED_LOG_F(error, LOG_ERROR, )
#endif

#undef SPECIALIZED_LOG_F

    // --- BOOKMARKED LOG FUNCTIONS ---

    void log_mark_log(context_t *context, enum loglevel_e level, struct bookmark_s bookmark, const char *fmt, ...)
{
    va_list vlist;
    va_start(vlist, fmt);
    logtarget_dispatch(context, level, bookmark, fmt, vlist);
    va_end(vlist);
}

// generating all the specialized one from the same definition
#define SPECIALIZED_LOG_MARK_F(name, NAME, post)                                               \
    void log_mark_##name(context_t *context, struct bookmark_s bookmark, const char *fmt, ...) \
    {                                                                                          \
        va_list vlist;                                                                         \
        va_start(vlist, fmt);                                                                  \
        logtarget_dispatch(context, NAME, bookmark, fmt, vlist);                               \
        va_end(vlist);                                                                         \
        post;                                                                                  \
    }

SPECIALIZED_LOG_MARK_F(trace, LOG_TRACE, )
SPECIALIZED_LOG_MARK_F(debug, LOG_DEBUG, )
SPECIALIZED_LOG_MARK_F(pedantic, LOG_PEDANTIC, )
SPECIALIZED_LOG_MARK_F(warning, LOG_WARNING, )

#ifdef __GNUC__
__attribute__((noreturn)) SPECIALIZED_LOG_MARK_F(error, LOG_ERROR, __builtin_unreachable())
#else
SPECIALIZED_LOG_MARK_F(error, LOG_ERROR, )
#endif

#undef SPECIALIZED_LOG_MARK_F
