/**
 * @file filemanager.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Interface to the file manager
 * @version 0.1
 * @date 2022-03-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _FILEMANAGER_H
#define _FILEMANAGER_H

#include <stdio.h>

#include "directives.h"

/**
 * @brief Contain the file manager
 */
typedef struct pp_filemanager_s pp_filemanager_t;

/**
 * @brief Contain an open translation unit
 *
 * Will seamlessly follow #include directives
 */
typedef struct pp_tsunit_s pp_tsunit_t;

/**
 * @brief get the next directive from the translation unit
 *
 * @param context the context in which the directive is required
 * @param tsunit the translation unit giving the directive
 * @return struct pp_directive_s* the directive given, or NULL for end of input
 */
struct pp_directive_s *pp_tsunit_get(context_t *context, pp_tsunit_t *tsunit);

/**
 * @brief Close the translation unit
 *
 * @param tsunit the translation unit to close
 * @param recursive_close if the underlying sources need to be closed
 */
void pp_tsunit_close(pp_tsunit_t *tsunit, bool recursive_close);

/**
 * @brief Open a new file manager
 *
 * @param context the context opening it
 * @param fopen_f the funtion used to open a file
 * @return pp_filemanager_t* the opened file manager
 */
pp_filemanager_t *pp_filemanager_open(context_t *context, FILE *(*fopen_f)(context_t *context, char *fname, bool is_angled));

/**
 * @brief open a translation unit
 *
 * @param context the context needing the unit
 * @param filemanager the filemanager
 * @return pp_tsunit_t* the translation unit opened
 */
pp_tsunit_t *pp_tsunit_open(context_t *context, pp_filemanager_t *filemanager);

#endif // _FILEMANAGER_H