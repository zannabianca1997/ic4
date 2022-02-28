/*
    This file provide a simple struct to keep track of where a piece of code came from
*/

#ifndef _BOOKMARK_H
#define _BOOKMARK_H

#include <stddef.h> // size_t
#include <stdbool.h>

#ifdef BOOKMARK_CMP_NAMES
#include <string.h>
#endif

// contain a bookmark information
// row and col are 1-based
// NULL and 0 are used to mark missing information
struct bookmark_s
{
    const char *filename;
    size_t row;
    size_t col;
};

static inline struct bookmark_s bookmark_new(const char *filename, size_t row, size_t col)
{
    return (struct bookmark_s){
        .filename = filename,
        .row = row,
        .col = col};
}

static inline struct bookmark_s bookmark_startof(const char *filename)
{
    return (struct bookmark_s){
        .filename = filename,
        .row = 1,
        .col = 1};
}

static inline void bookmark_advance(struct bookmark_s *bookmark)
{
    bookmark->col++;
}
static inline void bookmark_newline(struct bookmark_s *bookmark)
{
    bookmark->row++;
    bookmark->col = 1;
}

static inline void bookmark_update(struct bookmark_s *bookmark, char ch)
{
    if (ch == '\n')
        bookmark_newline(bookmark);
    else
        bookmark_advance(bookmark);
}

enum compare_method_e
{
    CMP_EXACT,      // info must be present and equals or both missing
    CMP_COMPATIBLE, // info must be equals if both present. If one is missing is considered ok
    CMP_IGNORE      // info is ignored
};

/**
 * @brief Compare two bookmarks
 *
 * @param a the first bookmark
 * @param b the second bookmark
 *
 * @param cmp_names possible only if BOOKMARK_CMP_NAMES is defined. Compare method for file name
 * @param cmp_rows Compare method for row number
 * @param cmp_cols Compare method for col number
 *
 * @return true for successfull match
 * @return false for unsuccessfull match
 */
static inline bool bookmark_cmp(struct bookmark_s a, struct bookmark_s b,
#ifdef BOOKMARK_CMP_NAMES
                                enum compare_method_e cmp_names,
#endif
                                enum compare_method_e cmp_rows,
                                enum compare_method_e cmp_cols)
{
#ifdef BOOKMARK_CMP_NAMES
    switch (cmp_names)
    {
    case CMP_EXACT:
        if (a.filename != NULL && b.filename != NULL)
        {
            if (strcmp(a.filename, b.filename) != 0)
                return false;
        }
        else if (a.filename != NULL || b.filename != NULL)
            return false;
        break;
    case CMP_COMPATIBLE:
        if (a.filename != NULL && b.filename != NULL &&
            strcmp(a.filename, b.filename) != 0)
            return false;
        break;
    case CMP_IGNORE:
        break;
    }
#endif
    switch (cmp_rows)
    {
    case CMP_EXACT:
        if (a.row != 0 && b.row != 0)
        {
            if (a.row != b.row)
                return false;
        }
        else if (a.row != 0 || b.row != 0)
            return false;
        break;
    case CMP_COMPATIBLE:
        if (a.row != 0 && b.row != 0 &&
            a.row != b.row)
            return false;
        break;
    case CMP_IGNORE:
        break;
    }
    switch (cmp_cols)
    {
    case CMP_EXACT:
        if (a.col != 0 && b.col != 0)
        {
            if (a.col != b.col)
                return false;
        }
        else if (a.col != 0 || b.col != 0)
            return false;
        break;
    case CMP_COMPATIBLE:
        if (a.col != 0 && b.col != 0 &&
            a.col != b.col)
            return false;
        break;
    case CMP_IGNORE:
        break;
    }

    return true;
}

#endif // _BOOKMARK_H