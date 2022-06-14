#include <stdio.h>

#include "tokenizer.h"

int main(int argc, char const *argv[])
{
    printf("size of pp_token: %lu\n", sizeof(struct pp_token));
    return 0;
}
