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
         * @brief The directive collected up to here. NULL terminated if file is ended
         */
        struct pp_directive_s **collected_directives;
        /**
         * @brief The last collected directive.
         *
         * The array pointed from collected_directives is at least num_collected long.
         * if collected_directives[num_collected-1] == NULL the file is ended.
         */
        size_t num_collected;
        /**
         * @brief The number of directive for which space is prepared
         */
        size_t collected_size;
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
    /**
     * @brief the filemanager for this file
     */
    struct pp_filemanager_s *filemanager;
    /**
     * @brief the entry this file is referencing to
     */
    struct filetable_entry_s *filetable_entry;
    /**
     * @brief The directive that was returned last
     * Last directive returned is filetable_entry->collected_directives[directives_returned-1].
     * If directives_returned == filetable_entry->num_collected and a new directive is required a new directive must be collected.
     */
    size_t directives_returned;
};

/**
 * @brief Get another directive from a file
 *
 * @param context the context requiring the directive
 * @param filemanager the filemanager managing that file
 * @param filetable_entry the entry for that file
 */
static void collect_new_directive(context_t *context, struct pp_filemanager_s *filemanager, struct filetable_entry_s *filetable_entry)
{
    // TODO
}