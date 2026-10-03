/* crash.c - Segmentation Fault 로 죽는 프로그램. Core Dump 를 gdb 로 분석한다
 * 사용법: ulimit -c unlimited; ./crash
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct user {
    int   id;
    char *name;
};

static struct user *find_user(struct user *tbl, int n, int id)
{
    int i;

    for (i = 0; i < n; i++)
        if (tbl[i].id == id)
            return &tbl[i];
    return NULL;                                        /* 못 찾으면 NULL */
}

static size_t name_length(const struct user *u)
{
    return strlen(u->name);                             /* u 가 NULL 이면 여기서 죽는다 */
}

static void report(struct user *tbl, int n, int id)
{
    struct user *u = find_user(tbl, n, id);

    printf("user %d: name length = %zu\n", id, name_length(u));    /* NULL 검사를 빼먹었다 */
}

int main(void)
{
    struct user tbl[] = { { 1, "kim" }, { 2, "lee" }, { 3, "park" } };
    int         ids[] = { 1, 3, 7, 2 }, i;

    for (i = 0; i < 4; i++)
        report(tbl, 3, ids[i]);
    return EXIT_SUCCESS;
}
