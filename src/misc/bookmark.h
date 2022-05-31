/**
 * @file bookmark.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Provide a basic struct to track source origin
 * @version 0.1
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _BOOKMARK_H
#define _BOOKMARK_H

#include <stddef.h>

/**
 * @brief Track a specific point from a source.
 *
 * Row and col are 1-based, like most editors. Missing data are marked by 0,0 and void
 */
struct bookmark
{
    size_t row;
    size_t col;
};

inline static void bm_advance(struct bookmark *mark)
{
    mark->col++;
}
inline static void bm_newline(struct bookmark *mark)
{
    mark->row++;
    mark->col = 1;
}
inline static void bm_count(struct bookmark *mark, char ch)
{
    if (ch != '\n')
        bm_advance(mark);
    else
        bm_newline(mark);
}

#endif // _BOOKMARK_H