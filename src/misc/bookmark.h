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

/**
 * @brief Track a specific point from a source.
 *
 * Row and col are 1-based, like most editors. Missing data are marked by 0,0 and void
 */
struct bookmark
{
    unsigned row;
    unsigned col;
    const char *source;
};
