/*
    This file provide a simple struct to keep track of where
    a piece of code come frome
*/

#ifndef _BOOKMARK_H
#define _BOOKMARK_H

struct bookmark_s
{
    const char *filename;
    int row;
    int col;
};

static inline struct bookmark_s bookmark_new(const char *filename)
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