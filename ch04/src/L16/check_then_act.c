/* check_then_act.c - 변수 하나하나는 문제없어 보여도 "검사 후 실행" 사이에 끼어들면 깨진다
 * 재고가 1 개인데 두 Thread 가 모두 "구매 성공" 한다
 */
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

static int stock = 1;
static int sold;
static pthread_barrier_t start;                /* 두 Thread 를 동시에 출발시킨다 */

static void *buyer(void *arg)
{
    pthread_barrier_wait(&start);
    if (stock > 0) {                        /* check */
        sched_yield();                      /* 끼어들 틈을 일부러 넓힌다 */
        stock--;                            /* act   */
        __atomic_fetch_add(&sold, 1, __ATOMIC_SEQ_CST);
        printf("buyer %ld: 구매 성공\n", (long)arg);
    } else {
        printf("buyer %ld: 품절\n", (long)arg);
    }
    return NULL;
}

int main(void)
{
    pthread_t a, b;

    pthread_barrier_init(&start, NULL, 2);
    pthread_create(&a, NULL, buyer, (void *)1L);
    pthread_create(&b, NULL, buyer, (void *)2L);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("sold = %d, stock = %d\n", sold, stock);
    return EXIT_SUCCESS;
}
