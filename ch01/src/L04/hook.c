/* hook.c - LD_PRELOAD 로 Library 함수를 가로챈다 (libhook.so) */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>

/* 1. 완전히 바꿔치기: 항상 같은 값 */
int rand(void)
{
    return 5;       /* rand() % 6 + 1 == 6 */
}

/* 2. 감싸기: 원래 함수를 찾아서 호출하고 앞뒤에 동작을 추가한다 */
int puts(const char *s)
{
    static int (*real_puts)(const char *);

    if (real_puts == NULL) {
        /* RTLD_NEXT: 검색 순서에서 "나 다음" Library 의 puts (= libc) */
        *(void **)(&real_puts) = dlsym(RTLD_NEXT, "puts");
        if (real_puts == NULL)
            return EOF;
    }
    fprintf(stderr, "[hook] puts(\"%s\")\n", s);
    return real_puts(s);
}
