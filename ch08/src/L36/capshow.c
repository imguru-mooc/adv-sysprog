/* capshow.c - 이 Process 의 Capability 다섯 집합을 /proc/self/status 에서 읽어 이름으로 풀어 보여 준다
 * 사용법: ./capshow          sudo ./capshow
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <unistd.h>

static void decode(const char *label, unsigned long long bits)
{
    int i, n = 0;

    printf("%-7s %016llx  ", label, bits);
    if (bits == 0) {
        printf("(없음)\n");
        return;
    }
    for (i = 0; i <= CAP_LAST_CAP; i++)
        if (bits & (1ULL << i))
            n++;
    if (n > 8) {
        printf("(%d 개: 사실상 전부)\n", n);
        return;
    }
    for (i = 0; i <= CAP_LAST_CAP; i++)
        if (bits & (1ULL << i)) {
            char *name = cap_to_name(i);

            printf("%s ", name);
            cap_free(name);
        }
    putchar('\n');
}

int main(void)
{
    char               line[256];
    unsigned long long v;
    FILE              *fp = fopen("/proc/self/status", "r");

    if (fp == NULL)
        return EXIT_FAILURE;
    printf("uid=%d euid=%d\n", (int)getuid(), (int)geteuid());
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "CapInh: %llx", &v) == 1) decode("CapInh", v);
        if (sscanf(line, "CapPrm: %llx", &v) == 1) decode("CapPrm", v);
        if (sscanf(line, "CapEff: %llx", &v) == 1) decode("CapEff", v);
        if (sscanf(line, "CapBnd: %llx", &v) == 1) decode("CapBnd", v);
        if (sscanf(line, "CapAmb: %llx", &v) == 1) decode("CapAmb", v);
        if (strncmp(line, "NoNewPrivs:", 11) == 0)  printf("%s", line);
    }
    fclose(fp);
    return EXIT_SUCCESS;
}
