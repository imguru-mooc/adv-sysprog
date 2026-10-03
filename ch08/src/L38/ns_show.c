/* ns_show.c - Process 가 속한 Namespace 들을 출력한다. 같은 번호 = 같은 Namespace
 * 사용법: ./ns_show [pid ...]      (인자가 없으면 자기 자신과 PID 1)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *kinds[] = { "mnt", "uts", "ipc", "pid", "net", "user", "cgroup", "time" };

static void show(const char *pid)
{
    char   path[64], link[64];
    size_t i;

    printf("PID %-8s", pid);
    for (i = 0; i < sizeof(kinds) / sizeof(kinds[0]); i++) {
        ssize_t n;

        snprintf(path, sizeof(path), "/proc/%s/ns/%s", pid, kinds[i]);
        n = readlink(path, link, sizeof(link) - 1);     /* 예: "pid:[4026531836]" */
        if (n < 0) {
            printf(" %s:?", kinds[i]);
            continue;
        }
        link[n] = '\0';
        printf(" %s", link);
    }
    putchar('\n');
}

int main(int argc, char *argv[])
{
    int i;

    if (argc < 2) {
        show("self");
        show("1");
        return EXIT_SUCCESS;
    }
    for (i = 1; i < argc; i++)
        show(argv[i]);
    return EXIT_SUCCESS;
}
