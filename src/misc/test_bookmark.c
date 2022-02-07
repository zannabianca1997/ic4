#include "bookmark.h"

const char *test_advance()
{
    struct bookmark_s mark = bookmark_new(NULL, 32, 42);
    bookmark_advance(&mark);
    if (mark.filename != NULL)
        return "Filename has changed";
    if (mark.row != 32)
        return "Row has changed";
    if (mark.col != 43)
    {
        if (mark.col == 42)
            return "Col has not changed";
        else
            return "Col has changed of the wrong amount";
    }
    return NULL;
}

const char *test_newline()
{
    struct bookmark_s mark = bookmark_new(NULL, 32, 42);
    bookmark_newline(&mark);
    if (mark.filename != NULL)
        return "Filename has changed";
    if (mark.col != 1)
        return "Col has not returned to first";
    if (mark.row != 33)
    {
        if (mark.row == 32)
            return "Row has not changed";
        else
            return "Row has changed of the wrong amount";
    }
    return NULL;
}

const char *test_update_ch()
{
    struct bookmark_s mark = bookmark_new(NULL, 32, 42);
    bookmark_update(&mark, 'h');
    if (mark.filename != NULL)
        return "Filename has changed";
    if (mark.row != 32)
        return "Row has changed";
    if (mark.col != 43)
    {
        if (mark.col == 42)
            return "Col has not changed";
        else
            return "Col has changed of the wrong amount";
    }
    return NULL;
}

const char *test_update_nl()
{
    struct bookmark_s mark = bookmark_new(NULL, 32, 42);
    bookmark_update(&mark, '\n');
    if (mark.filename != NULL)
        return "Filename has changed";
    if (mark.col != 1)
        return "Col has not returned to first";
    if (mark.row != 33)
    {
        if (mark.row == 32)
            return "Row has not changed";
        else
            return "Row has changed of the wrong amount";
    }
    return NULL;
}