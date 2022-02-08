#define _GNU_SOURCE // needed for fmemopen

#include <stdio.h>
#include <string.h>

#include "lines.h"
#include "linesco.h"

#define REPORT_MSG_MAX_LEN 256
#define LINES_BUF_MAX_LEN 256

static const struct
{
    char name[5];
    char nl[3];
} NEWLINES[] = {{"\\n", "\n"},
                {"\\r", "\r"},
                {"\\r\\n", "\r\n"},
                {"", ""}};

static const char *const TEST_LINES[] = {
    "Line1",
    "This is line 2 ",
    " This is another line",
    "",
    "Oh no, it's still going?",
    "Please kill me",
    NULL};

static char linebuf[LINES_BUF_MAX_LEN + 1];
static char report_msg[REPORT_MSG_MAX_LEN + 1];

const char *test_rawlines()
{
    context_t *testing_c = context_new(NULL, "testing rawlines splitting");

    // testing for all possible newlines
    for (size_t i = 0; NEWLINES[i].name[0] != '\0'; i++)
    {
        // start typing report
        char *report_msg_p1 = report_msg;
        report_msg_p1 += snprintf(report_msg_p1, REPORT_MSG_MAX_LEN - (report_msg_p1 - report_msg), "Testing newline '%s': ", NEWLINES[i].name);

        // preparing test case
        linebuf[0] = '\0';
        strcat(linebuf, TEST_LINES[0]);
        for (size_t j = 1; TEST_LINES[j] != NULL; j++)
        {
            strcat(linebuf, NEWLINES[i].nl);
            strcat(linebuf, TEST_LINES[j]);
        }
        FILE *input = fmemopen(linebuf, strlen(linebuf), "r");

        // creating a line stream
        linestream_t *lstream = linestream_open(testing_c, input);

        // iterate to the end of the stream
        size_t line_count = 0;
        for (struct logical_line_s *ll = linestream_get(testing_c, lstream); ll != NULL; line_free(ll), ll = linestream_get(testing_c, lstream))
        {
            line_count++;

            char *report_msg_p2 = report_msg_p1;
            report_msg_p2 += snprintf(report_msg_p2, REPORT_MSG_MAX_LEN - (report_msg_p2 - report_msg), "Line %lu ", line_count);

            // check line count
            if (ll->index[0].row != line_count)
            {
                snprintf(report_msg_p2, REPORT_MSG_MAX_LEN - (report_msg_p2 - report_msg), "was reported as line %lu", ll->index[0].row);
                return report_msg;
            }
            // check line start
            if (ll->index[0].start != 0)
            {
                snprintf(report_msg_p2, REPORT_MSG_MAX_LEN - (report_msg_p2 - report_msg), "started at %lu instead of 0", ll->index[0].start);
                return report_msg;
            }
            // check is a single line
            if (ll->index[1].row != 0)
            {
                size_t rl_count = 1;
                while (ll->index[rl_count].row != 0)
                    rl_count++;

                snprintf(report_msg_p2, REPORT_MSG_MAX_LEN - (report_msg_p2 - report_msg), "has %lu raw lines inside (1 expected)", rl_count);
                return report_msg;
            }
            // check we did not go over the limit
            if (TEST_LINES[line_count - 1] == NULL)
            {
                snprintf(report_msg, REPORT_MSG_MAX_LEN, "Exceeded %lu lines, first unexpected line has content \"%s\"", line_count, ll->content);
                return report_msg;
            }
            // check content
            if (strcmp(ll->content, TEST_LINES[line_count - 1]) != 0)
            {
                snprintf(report_msg_p2, REPORT_MSG_MAX_LEN - (report_msg_p2 - report_msg), "was expected to be \"%s\", found instead \"%s\"", TEST_LINES[line_count - 1], ll->content);
                return report_msg;
            }
        }
    }
    return NULL;
}

static const char *const TEST_MERGING_LINES[] = {
    "Line1",
    "This is line 2 \\",
    " This is another line\\",
    "\\",
    "Oh no, it's still going?",
    "Please kill me",
    NULL};

// how many components should be in every logical line
static const unsigned LOGICAL_LINES_COMPONENTS[] = {1, 4, 1};
// how many components should be in every logical line
static const unsigned LOGICAL_LINES_STARTS[] = {1, 2, 6};
// whats inside each?
static const char *const LOGICAL_LINES_CONTENT[] = {
    "Line1",
    "This is line 2  This is another lineOh no, it's still going?",
    "Please kill me"};

const char *test_rawline_merge()
{
    context_t *testing_c = context_new(NULL, "testing rawlines merging");

    // testing for all possible newlines
    for (size_t i = 0; NEWLINES[i].name[0] != '\0'; i++)
    {
        // start typing report
        char *report_msg_p1 = report_msg;
        report_msg_p1 += snprintf(report_msg_p1, REPORT_MSG_MAX_LEN - (report_msg_p1 - report_msg), "Testing newline '%s': ", NEWLINES[i].name);

        // preparing test case
        linebuf[0] = '\0';
        strcat(linebuf, TEST_MERGING_LINES[0]);
        for (size_t j = 1; TEST_MERGING_LINES[j] != NULL; j++)
        {
            strcat(linebuf, NEWLINES[i].nl);
            strcat(linebuf, TEST_MERGING_LINES[j]);
        }

        FILE *input = fmemopen(linebuf, strlen(linebuf), "r");

        // creating a line stream
        linestream_t *lstream = linestream_open(testing_c, input);

        // iterate to the end of the stream
        size_t line_count = 0;
        for (struct logical_line_s *ll = linestream_get(testing_c, lstream); ll != NULL; line_free(ll), ll = linestream_get(testing_c, lstream))
        {
            line_count++;
            fprintf(stderr, "%lu >> %s\n", line_count, ll->content);
            fflush(stderr);

            char *report_msg_p2 = report_msg_p1;
            report_msg_p2 += snprintf(report_msg_p2, REPORT_MSG_MAX_LEN - (report_msg_p2 - report_msg), "Line %lu ", line_count);

            // check line count
            if (ll->index[0].row != LOGICAL_LINES_STARTS[line_count - 1])
            {
                snprintf(report_msg_p2, REPORT_MSG_MAX_LEN - (report_msg_p2 - report_msg), "was reported as line %lu, instead of %u", ll->index[0].row, LOGICAL_LINES_STARTS[line_count - 1]);
                return report_msg;
            }
            // check number of lines
            size_t rl_count = 1;
            while (ll->index[rl_count].row != 0)
                rl_count++;
            if (rl_count != LOGICAL_LINES_COMPONENTS[line_count - 1])
            {
                snprintf(report_msg_p2, REPORT_MSG_MAX_LEN - (report_msg_p2 - report_msg), "has %lu raw lines inside (%u expected)", rl_count, LOGICAL_LINES_COMPONENTS[line_count - 1]);
                return report_msg;
            }
            // TODO: Check line position inside string
            // check content
            if (strcmp(ll->content, LOGICAL_LINES_CONTENT[line_count - 1]) != 0)
            {
                snprintf(report_msg_p2, REPORT_MSG_MAX_LEN - (report_msg_p2 - report_msg), "was expected to be \"%s\", found instead \"%s\"", LOGICAL_LINES_CONTENT[line_count - 1], ll->content);
                return report_msg;
            }
        }
    }
    return NULL;
}