#include <stdio.h>

#include "tap.h"

static void default_putc(void *stream, char ch) { putc(ch, (FILE *)stream); }

int main(int argc, char const *argv[])
{
    set_output(&default_putc, (void *)stdout);

    plan(4, NULL);

    return 0;
}
