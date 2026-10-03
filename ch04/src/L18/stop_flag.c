/* stop_flag.c - "flag 를 1 로 바꿨는데 Thread 가 멈추지 않는다"
 * 일반 변수를 Thread 간 신호로 쓰면 Compiler 가 Loop 밖으로 읽기를 빼낼 수 있다(-O2).
 * -DUSE_ATOMIC 으로 Build 하면 atomic_bool 을 사용한다.
 * 멈추지 않는 경우를 위해 3 초 뒤 alarm 으로 강제 종료한다.
 */
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef USE_ATOMIC
static atomic_bool stop;
#define READ_STOP() atomic_load(&stop)
#define SET_STOP()  atomic_store(&stop, 1)
#else
static int stop;                            /* 일반 변수 */
#define READ_STOP() stop
#define SET_STOP()  (stop = 1)
#endif

static void *worker(void *arg)
{
    unsigned long spins = 0;

    (void)arg;
    while (!READ_STOP())
        spins++;
    printf("worker: stop 을 확인하고 종료 (spins=%lu)\n", spins);
    return NULL;
}

int main(void)
{
    pthread_t tid;

    alarm(3);                               /* 안전장치: 3 초 뒤 SIGALRM 으로 종료 */
    if (pthread_create(&tid, NULL, worker, NULL) != 0)
        return EXIT_FAILURE;
    sleep(1);
    SET_STOP();
    printf("main  : stop = 1 로 설정, worker 를 기다립니다...\n");
    fflush(stdout);
    pthread_join(tid, NULL);
    printf("main  : 정상 종료\n");
    return EXIT_SUCCESS;
}
