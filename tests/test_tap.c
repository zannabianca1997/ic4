#include <stdio.h>

#include "tap.h"

int main(int argc, char const *argv[])
{
    plan(2, NULL);

    diagnostic("This is a test diagnostic message");

    ok(3 > 2, "basic comparison");

    todo("Some todo");
    fail("Oh no...");
    end_todo();

    return exit_status();
}
