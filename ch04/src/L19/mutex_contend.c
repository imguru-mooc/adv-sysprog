/* mutex_contend.c - 여러 Thread 가 하나의 mutex 를 두고 경쟁한다. Critical Section 길이를 바꿀 수 있다
 * 사용법: ./mutex_contend [thread 수=4] [critical section 안에서의 작업량=200]
 * 확인  : strace -f -c -e trace=futex ./mutex_contend
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static volatile long   shared;
static long            loops = 200000;
static int             work  = 200;

static void *worker(void *arg)
{
    long i;
    int  k;

    (void)arg;
    for (i = 0; i < loops; i++) {
        pthread_mutex_lock(&m);
        for (k = 0; k < work; k++)          /* Lock 을 잡고 있는 시간 */
            shared++;
        pthread_mutex_unlock(&m);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t tid[64];
    int       i, n = (argc > 1) ? atoi(argv[1]) : 4;

    if (argc > 2)
        work = atoi(argv[2]);
    if (n < 1 || n > 64)
        return EXIT_FAILURE;
    for (i = 0; i < n; i++)
        if (pthread_create(&tid[i], NULL, worker, NULL) != 0)
            return EXIT_FAILURE;
    for (i = 0; i < n; i++)
        pthread_join(tid[i], NULL);
    printf("threads %d, shared = %ld (expected %ld)\n", n, shared, (long)n * loops * work);
    return EXIT_SUCCESS;
}
