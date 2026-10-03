/* atomic_counter.c - C11 <stdatomic.h> 로 Lock 없이 정확한 Counter 를 만든다 */
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

static atomic_long counter;
static long        loops = 10000000;

static void *worker(void *arg)
{
    long i;

    (void)arg;
    for (i = 0; i < loops; i++)
        atomic_fetch_add(&counter, 1);      /* x86-64: lock add / lock xadd 명령 하나 */
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t tid[64];
    int       i, n = (argc > 1) ? atoi(argv[1]) : 2;

    if (n < 1 || n > 64)
        return EXIT_FAILURE;
    for (i = 0; i < n; i++)
        if (pthread_create(&tid[i], NULL, worker, NULL) != 0)
            return EXIT_FAILURE;
    for (i = 0; i < n; i++)
        pthread_join(tid[i], NULL);
    printf("expected %ld\nactual   %ld\n", (long)n * loops, atomic_load(&counter));
    return EXIT_SUCCESS;
}
