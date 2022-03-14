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
struct pp_directive_s *pp_file_get(context_t *context, pp_file_t *file);

/**
 * @brief Close the file
 *
 * @param file the file to close
 * @param recursive_close if the underlying sources need to be closed
 */
void pp_file_close(pp_file_t *file, bool recursive_close);

/**
 * @brief Contain some data to identify a file
 */
typedef void fileid_t;

/**
 * @brief The type of function used to find a file
 *
 * @param context the context in which the file is needed
 * @param fname the name of the file
 * @param is_angled if the file is to search in angled mode (internal header are included)
 * @return fileid_t * a pointer to the file identifier
 */
typedef fileid_t *find_file_t(context_t *context, char const *fname, bool is_angled);

/**
 * @brief Type of function used to sort the fileids.
 *
 * Must be 0 if and only if the files are the same.
 * Must be transitive:
 *     (*fileid_cmp)(a,b) > 0 && (*fileid_cmp)(b,c) > 0
 * implies
 *     (*fileid_cmp)(a,b) > 0
 *
 * @param a the first fileid
 * @param b the second fileid
 * @return int the result of the comparison
 */
typedef int fileid_cmp_t(fileid_t *a, fileid_t *b);

/**
 * @brief The typ ae of function used to open a file
 *
 * @param context the context in which the file is needed
 * @param fileid the id of the file
 * @return FILE * a readable stream pointed to the start of the file
 */
typedef FILE *open_file_t(context_t *context, fileid_t *fileid);

/**
 * @brief Open a new file manager
 *
 * @param context the context opening it
 * @param find_file The function used to find a file
 * @param fileid_cmp The function used to compare two files
 * @param open_file The function used to open a file
 * @return pp_filemanager_t* the opened file manager
 */
pp_filemanager_t *pp_filemanager_open(context_t *context,
                                      find_file_t *find_file,
                                      fileid_cmp_t *fileid_cmp,
                                      open_file_t *open_file);

/**
 * @brief open a file
 *
 * @param context the context needing the unit
 * @param filemanager the filemanager
 * @param fname the name of the file
 * @param is_angled if the file is opened in angled mode
 * @return pp_file_t* the file opened
 */
pp_file_t *pp_file_open(context_t *context, pp_filemanager_t *filemanager, char const *fname, bool is_angled);

#endif // _FILEMANAGER_H