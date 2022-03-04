/*
    This file define an enum used to indicate the seriusness of a log message
*/

#ifndef _LOGLEVEL_H
#define _LOGLEVEL_H

enum loglevel_e {
    // Everything that the application do
    LOG_TRACE = 0,
    // Small stuff that can be important
    LOG_DEBUG = 1,
    // Small errors that the program can pass over
    LOG_PEDANTIC = 2,
    // Unclear what to do, the program is guessing
    LOG_WARNING = 3,
    // Error that the program cannot surpass
    // Logging at this level is guarantee to not return
    LOG_ERROR = 4
};

// TODO: Add and implement LOG_CRITICAL and LOG_INFO

#define LOG_LEVEL_NAME_MAX_LEN 9
static const char LOG_LEVEL_NAME[][LOG_LEVEL_NAME_MAX_LEN] = {
    "trace",
    "debug",
    "pedantic",
    "warning",
    "error"
}

#endif // _LOGLEVEL_H