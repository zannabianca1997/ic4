/**
 * @file filemanager.c
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Manage the file opens
 * @version 0.1
 * @date 2022-03-14
 *
 * @copyright Copyright (c) 2022
 *
 * Read the file requested.
 * Mark the directive with the file from which they came.
 * When asked again for the same file, it won't reread it.
 *
 */

#include "filemanager.h"
#include "directives.h"

/**
 * @brief All the data on a filemanager
 *
 */
struct pp_filemanager_s
{
    /**
     * @brief the table of the open files
     *
     */
    struct filetable_entry_s
    {
        /**
         * @brief the id of the file
         */
        fileid_t *id;
        /**
         * @brief The directive collected up to here
         */
        struct pp_directive_s **collected_directives;
        /**
         * @brief the directive stream
         */
        directive_stream_t *source;
    } * file_table;

    // file managing functions

    find_file_t *find_file;
    fileid_cmp_t *fileid_cmp;
    open_file_t *open_file;
};

struct pp_file_s
{
    struct filetable_entry_s *filetable_entry;
    size_t current_idx;
};
