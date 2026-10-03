/* greet.c - 직접 만드는 Shared Library (libgreet.so) */
#include <stdio.h>
#include "greet.h"

static int count;

void greet(const char *name)
{
    count++;
    printf("[libgreet] hello, %s (%d)\n", name, count);
}

int greet_count(void)
{
    return count;
}
