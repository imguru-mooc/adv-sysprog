/* libonly.c - System Call 없이 User Space 안에서만 끝나는 Library 함수 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    const char *s = (argc > 1) ? argv[1] : "user space only";
    volatile size_t total = 0;
    int i;

    /* strlen, strchr, abs 는 Kernel의 도움이 필요 없다 */
    for (i = 0; i < 1000000; i++)
        total += strlen(s) + (strchr(s, 'x') != NULL) + (size_t)abs(-i);

    return (int)(total & 0x7f);
}
