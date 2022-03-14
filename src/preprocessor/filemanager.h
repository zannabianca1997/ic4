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
 * @brief Contain an open file
 *
 */
typedef struct pp_file_s pp_file_t;

/**
 * @brief get the next directive from the file
 *
 * @param context the context in which the directive is required
 * @param file the file giving the directive
 * @return struct pp_directive_s* the directive given, or NULL for end of input
 */
struct pp_directive_s *pp_tsunit_get(context_t *context, pp_file_t *file);

/**
 * @brief Close the file
 *
 * @param file the file to close
 * @param recursive_close if the underlying sources need to be closed
 */
void pp_file_close(pp_file_t *tsunit, bool recursive_close);

/**
 * @brief Open a new file manager
 *
 * @param context the context opening it
 * @param fopen_f the funtion used to open a file
 * @return pp_filemanager_t* the opened file manager
 */
pp_filemanager_t *pp_filemanager_open(
    context_t *context,
    /**
     * @brief The funtion used to open a file
     *
     * @param context the context in which the file is needed
     * @param fname the name of the file
     * @param is_angled if the file is to search in angled mode (internal header are included)
     * @return FILE * a readable stream pointed to the start of the file
     */
    FILE *(*fopen_f)(context_t *context, char const *fname, bool is_angled));

/**
 * @brief open a file
 *
 * @param context the context needing the unit
 * @param filemanager the filemanager
 * @return pp_file_t* the file opened
 */
pp_file_t *pp_file_open(context_t *context, pp_filemanager_t *filemanager);

#endif // _FILEMANAGER_H