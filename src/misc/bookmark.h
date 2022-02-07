/*
    This file provide a simple struct to keep track of where a piece of code came from
*/

#ifndef _BOOKMARK_H
#define _BOOKMARK_H

#include <stddef.h> // size_t

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

#endif // _BOOKMARK_H