/* deadlock.c - 멈춰 버린 프로그램. 죽이지 않고 gdb -p 로 붙어서 원인을 찾는다
 *   Thread A: lock(m1) → lock(m2)      Thread B: lock(m2) → lock(m1)
 */
#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static pthread_mutex_t m_account = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m_log     = PTHREAD_MUTEX_INITIALIZER;

static void *transfer(void *arg)
{
    (void)arg;
    pthread_setname_np(pthread_self(), "transfer");
    pthread_mutex_lock(&m_account);
    usleep(100 * 1000);
    pthread_mutex_lock(&m_log);                         /* audit 이 m_log 를 쥐고 있다 */
    pthread_mutex_unlock(&m_log);
    pthread_mutex_unlock(&m_account);
    return NULL;
}

static void *audit(void *arg)
{
    (void)arg;
    pthread_setname_np(pthread_self(), "audit");
    pthread_mutex_lock(&m_log);
    usleep(100 * 1000);
    pthread_mutex_lock(&m_account);                     /* transfer 가 m_account 를 쥐고 있다 */
    pthread_mutex_unlock(&m_account);
    pthread_mutex_unlock(&m_log);
    return NULL;
}

int main(void)
{
    pthread_t a, b;

    printf("pid = %d  (멈추면 다른 Terminal 에서: sudo gdb -p %d)\n", (int)getpid(), (int)getpid());
    fflush(stdout);
    pthread_create(&a, NULL, transfer, NULL);
    pthread_create(&b, NULL, audit, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("완료\n");
    return EXIT_SUCCESS;
}
