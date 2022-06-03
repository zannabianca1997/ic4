/**
 * @file errors.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Provide error codes and data structure
 * @version 0.1
 * @date 2022-06-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _ERRORS_H
#define _ERRORS_H

#include <stdio.h>
#include <stdlib.h>

// TODO: when variadic arguments are into the compiler all this must go into functions

#define debug(...) fprintf(__VA_ARGS__)
#define log(...) fprintf(__VA_ARGS__)
#define warn(...) fprintf(stderr, __VA_ARGS__)
#define error(...) fprintf(stderr, __VA_ARGS__)
#define critical(...)                 \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    }

#endif