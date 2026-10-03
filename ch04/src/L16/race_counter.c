/* race_counter.c - 두 Thread 가 같은 변수를 보호 없이 증가시킨다
 * 사용법: ./race_counter [thread 수=2] [thread 당 반복=10000000]
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static long counter;                        /* 공유 변수 */
static long loops = 10000000;

static void *worker(void *arg)
{
    long i;

    (void)arg;
    for (i = 0; i < loops; i++)
        counter++;                          /* C 에서는 한 줄, CPU 에서는 세 단계 */
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t tid[64];
    int       i, n = (argc > 1) ? atoi(argv[1]) : 2;

    if (argc > 2)
        loops = atol(argv[2]);
    if (n < 1 || n > 64) {
        fprintf(stderr, "thread 수는 1~64\n");
        return EXIT_FAILURE;
    }
    for (i = 0; i < n; i++)
        if (pthread_create(&tid[i], NULL, worker, NULL) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    for (i = 0; i < n; i++)
        pthread_join(tid[i], NULL);

    printf("expected %ld\n", (long)n * loops);
    printf("actual   %ld   (lost %ld)\n", counter, (long)n * loops - counter);
    return EXIT_SUCCESS;
}
