/*
    This file provide an interface to set where the application is logging to.
    Every stream is containted in a logtarget, that has level for what to log and how
*/

#ifndef _LOGTARGET_H
#define _LOGTARGET_H

#include <stdio.h>
#include <stdbool.h>

#include "loglevel.h"

// --- MANAGING TARGETS ---

// contain the target of the logging
typedef struct logtarget_s logtarget_t;

struct logtarget_errorlevels_s
{
    enum loglevel_e debuglevel;
    enum loglevel_e warninglevel;
    enum loglevel_e errorlevel;
};

// default logging
static const struct logtarget_errorlevels_s DEFAULT_ERRORLEVELS = {
#ifdef DEBUG
    .debuglevel = LOG_DEBUG,
#else
    .debuglevel = LOG_WARNING,
#endif
    .warninglevel = LOG_WARNING,
    .errorlevel = LOG_ERROR};

// create a new target
logtarget_t *logtarget_new(context_t *context, FILE *target, struct logtarget_errorlevels_s errorlevels);

// remove a target (stop logging to it)
// second parameter specify if it will call fclose() on the underlying stream
// calling with anything but a value generated from logtarget_new will get undefinite behaviour
void logtarget_remove(context_t *context, logtarget_t *target, bool close_stream);

// --- SETTING ERRORLEVELS ---

// set all the loglevels of the target
void logtarget_set_errorlevels(context_t *context, logtarget_t *target, struct logtarget_errorlevels_s errorlevels);

// set the debuglevel of the target
// message under that level will be ignored
// message over or equal will be emitted as debug
// will emit an error if the level is over warninglevel
void logtarget_set_debuglevel(context_t *context, logtarget_t *target, enum loglevel_e new_debuglevel);

// set the warninglevel of the target
// message under that level will be shown as debug
// message over or equal will be emitted as warning
// will emit an error if the level is over errorlevel, or under debuglevel
void logtarget_set_warninglevel(context_t *context, logtarget_t *target, enum loglevel_e new_warninglevel);

// set the errorlevel of the target
// message under that level will be shown as warnings
// message over or equal will be emitted as errors, and application is stopped
// will emit an error if the level is over LOG_WARNING, or under warninglevel
void logtarget_set_errorlevel(context_t *context, logtarget_t *target, enum loglevel_e new_errorlevel);

#ifdef ANSI_FORMATTED_OUTPUT

// --- ANSI FORMATTED OUTPUT ---

// set if the given target should use ANSI escape sequence to format the output
void logtarget_set_use_ansi_formatting(context_t *context, logtarget_t *logtarget, bool use_ansi_fmt);

#ifndef __COUNTER__
#error "The macro __COUNTER__ is needed for automatic flag numbering"
#endif

enum logtarget_ansi_flag_e
{
    _LOGTARGET_ANSI_FLAG_ENUM_COUNTER_START = __COUNTER__,
#define AUTO_FLAG() (1 << (__COUNTER__ - 1 - _LOGTARGET_ANSI_FLAG_ENUM_COUNTER_START))

    // how the context text is formatted
    LOGTARGET_ANSI_FLAG_DEBUG_CONTEXT = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_WARNING_CONTEXT = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_ERROR_CONTEXT = AUTO_FLAG(),

    // how the context name(s) is formatted
    LOGTARGET_ANSI_FLAG_DEBUG_CONTEXT_NAME = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_WARNING_CONTEXT_NAME = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_ERROR_CONTEXT_NAME = AUTO_FLAG(),

    // how the bookmark is formatted
    LOGTARGET_ANSI_FLAG_DEBUG_BOOKMARK = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_WARNING_BOOKMARK = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_ERROR_BOOKMARK = AUTO_FLAG(),

    // how the bookmark nums (row and colums) are formatted
    LOGTARGET_ANSI_FLAG_DEBUG_BOOKMARK_NUMS = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_WARNING_BOOKMARK_NUMS = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_ERROR_BOOKMARK_NUMS = AUTO_FLAG(),

    // how the bookmark filename is formatted
    LOGTARGET_ANSI_FLAG_DEBUG_BOOKMARK_FILENAME = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_WARNING_BOOKMARK_FILENAME = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_ERROR_BOOKMARK_FILENAME = AUTO_FLAG(),

    // how the log text is formatted
    LOGTARGET_ANSI_FLAG_DEBUG_TEXT = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_WARNING_TEXT = AUTO_FLAG(),
    LOGTARGET_ANSI_FLAG_ERROR_TEXT = AUTO_FLAG(),

#undef AUTO_FLAG
};

struct logtarget_ansi_format_s
{
    enum logtarget_ansi_flag_e flags[256];
};

// set the colors the target should use for its output
void logtarget_set_ansi_format(context_t *context, logtarget_t *logtarget, struct logtarget_ansi_format_s *ansi_format);

#endif // ANSI_FORMATTED_OUTPUT

#endif // _LOGTARGET_H