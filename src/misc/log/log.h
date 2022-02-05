/*
    This file provide a common interface to log what the application is doing
*/

#ifndef _LOG_H
#define _LOG_H

#include "loglevel.h"
#include "../bookmark.h"
#include "../context/context.h"


// --- LOG FUNCTIONS ---

// log a message with the given errorlevel
void log_log(context_t *context, enum loglevel_e level, const char *fmt, ...);

// log a message with LOG_TRACE errorlevel
void log_trace(context_t *context, const char *fmt, ...);

// log a message with LOG_DEBUG errorlevel
void log_debug(context_t *context, const char *fmt, ...);

// log a message with LOG_PEDANTIC errorlevel
void log_pedantic(context_t *context, const char *fmt, ...);

// log a message with LOG_WARNING errorlevel
void log_warning(context_t *context, const char *fmt, ...);

// log a message with LOG_ERROR errorlevel.
// guarantee to abort
#ifdef __GNUC__
__attribute__((noreturn))
#endif
void log_error(context_t *context, const char *fmt, ...);

// --- BOOKMARKED LOG FUNCTIONS ---

// log a message with the given errorlevel and file mark reference
void log_mark_log(context_t *context, enum loglevel_e level, struct bookmark_s bookmark, const char *fmt, ...);

// log a message with LOG_TRACE errorlevel and file mark reference
void log_mark_trace(context_t *context, struct bookmark_s bookmark, const char *fmt, ...);

// log a message with LOG_DEBUG errorlevel and file mark reference
void log_mark_debug(context_t *context, struct bookmark_s bookmark, const char *fmt, ...);

// log a message with LOG_PEDANTIC errorlevel and file mark reference
void log_mark_pedantic(context_t *context, struct bookmark_s bookmark, const char *fmt, ...);

// log a message with LOG_WARNING errorlevel and file mark reference
void log_mark_warning(context_t *context, struct bookmark_s bookmark, const char *fmt, ...);

// log a message with LOG_ERROR errorlevel and file mark reference
// guarantee to abort
#ifdef __GNUC__
__attribute__((noreturn))
#endif
void log_mark_error(context_t *context, struct bookmark_s bookmark, const char *fmt, ...);

#endif // _LOG_H