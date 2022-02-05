/*
    Provide an interface to control a single loglevel, abortlevel.
    When a log is put of level greater than abortlevel the program will stop.
    An abort procedure can be set, and it will receive the context that caused the abort
*/

#ifndef _ABORTLEVEL_H
#define _ABORTLEVEL_H

#include "loglevel.h"
#include "../context/context.h"

// set the errorlevel at wich the program will abort
// setting it over LOG_LEVEL will result in undefinite beaviour
void set_abortlevel(context_t *context, enum loglevel_e new_abortlevel);

// set a funtion to be called when aborting
// program will close anyway after the function return
void set_abortfunc(void (*new_abortfun)(context_t *));

#endif // _ABORTLEVEL_H