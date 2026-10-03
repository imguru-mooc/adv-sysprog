/* false_sharing.c - 서로 다른 변수를 쓰는데도 느려진다: 같은 Cache Line 에 있기 때문
 * 사용법: ./false_sharing         (두 Counter 가 붙어 있음)
 *         ./false_sharing pad     (64 Byte 경계로 떨어뜨림)
 * CPU 가 2 개 이상이어야 차이가 난다.
 */
#include <pthread.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOOPS 200000000L

struct packed_pair { volatile long a; volatile long b; };                 /* 16 Byte: 한 Cache Line 안 */
struct padded_pair { alignas(64) volatile long a; alignas(64) volatile long b; };

static struct packed_pair packed;
static struct padded_pair padded;

static void *bump(void *arg)
{
    volatile long *p = arg;
    long           i;

    for (i = 0; i < LOOPS; i++)
        (*p)++;
    return NULL;
}

int main(int argc, char *argv[])
{
    int             pad = (argc > 1 && strcmp(argv[1], "pad") == 0);
    volatile long  *pa  = pad ? &padded.a : &packed.a;
    volatile long  *pb  = pad ? &padded.b : &packed.b;
    struct timespec t0, t1;
    pthread_t       ta, tb;

    printf("&a = %p\n&b = %p   (거리 %ld Byte)\n", (void *)pa, (void *)pb,
           (long)((char *)pb - (char *)pa));
    clock_gettime(CLOCK_MONOTONIC, &t0);
    pthread_create(&ta, NULL, bump, (void *)pa);
    pthread_create(&tb, NULL, bump, (void *)pb);
    pthread_join(ta, NULL);
    pthread_join(tb, NULL);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("a = %ld, b = %ld   %.0f ms\n", *pa, *pb,
           (double)(t1.tv_sec - t0.tv_sec) * 1e3 + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6);
    return EXIT_SUCCESS;
}
