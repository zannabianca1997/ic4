#include <stdio.h>

#include "misc/context/context.h"

#include "misc/log/log.h"
#include "misc/log/abortlevel.h"
#include "misc/log/logtarget.h"

#include "misc/bookmark.h"

int main()
{
    // creating main context
    context_t *maincont = context_new(NULL, "test_main");

    // open file1 for logging
    FILE *flog1 = fopen("debug/test_log_1.txt", "w");
    if (flog1 == NULL)
    {
        printf("Cannot opern logfile\n");
        return 1;
    }
    logtarget_t *filelog1 = logtarget_new(maincont, flog1, DEFAULT_ERRORLEVELS);
    logtarget_set_debuglevel(maincont, filelog1, LOG_TRACE);
    // open file2 for logging
    FILE *flog2 = fopen("debug/test_log_2.txt", "w");
    if (flog2 == NULL)
    {
        printf("Cannot opern logfile\n");
        return 1;
    }
    logtarget_t *filelog2 = logtarget_new(maincont, flog2, DEFAULT_ERRORLEVELS);

    // try logging
    context_t *subcont = context_new(maincont, "basic_testing");
    log_debug(subcont, "Hello from logging!");
    log_trace(subcont, "you will see this only in the file1... The answer is %d", 42);
    log_warning(subcont, "Umm.. this is a warn%s", "ing");
    context_free(subcont);

    // try bookmark logging
    subcont = context_new(maincont, "bookmark_testing");
    log_mark_debug(subcont, bookmark_startof("test_file.c"), "This has a bookmark!");
    log_mark_debug(subcont, bookmark_new("test_file.c", 42, 0), "This bookmark has no column");
    log_mark_debug(subcont, bookmark_new(NULL, 42, 21), "This bookmark has no file");
    log_mark_debug(subcont, bookmark_new("test_file.c", 0, 0), "This bookmark is only a file");
    log_mark_debug(subcont, bookmark_new(NULL, 23, 0), "This bookmark is only a row");
    log_mark_debug(subcont, bookmark_new(NULL, 0, 0), "This don't have a bookmark");
    context_free(subcont);

    // testing different logging importances
    subcont = context_new(maincont, "different_importances");
    logtarget_set_warninglevel(subcont, filelog2, LOG_PEDANTIC);
    log_pedantic(subcont, "This will be considere a warning by file2, only a debug by file1");
    context_free(subcont);

    // removing filelog2
    subcont = context_new(maincont, "removing_and_adding_targets");
    logtarget_remove(subcont, filelog2, false);
    log_warning(subcont, "This will appear only in the file1");
    filelog2 = logtarget_new(subcont, flog2, DEFAULT_ERRORLEVELS);
    context_free(subcont);

    // abort test
    if (true)
    {
        subcont = context_new(maincont, "aborting_test");
        log_error(subcont, "And this will abort the program!");
        context_free(subcont);
    }

    // delete targets
    logtarget_remove(maincont, filelog1, true);
    logtarget_remove(maincont, filelog2, true);

    return 0;
}
