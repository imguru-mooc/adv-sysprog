/* lockfree_stack.c - CAS 로 만드는 Lock-Free Stack (Treiber Stack)
 * 여러 Thread 가 동시에 push 하고, 이어서 동시에 pop 한다. mutex 가 없다.
 * Node 를 재사용하거나 free 하지 않으므로 ABA 문제가 생기지 않는 단순한 형태다.
 */
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#define NTHREADS 4
#define PER      250000

struct node { struct node *next; long value; };

static _Atomic(struct node *) top;
static struct node           *pool;
static atomic_long            popped_sum, popped_cnt;

static void push(struct node *n)
{
    struct node *old = atomic_load(&top);

    do {
        n->next = old;                      /* 1. 내가 본 top 을 next 로 삼고 */
    } while (!atomic_compare_exchange_weak(&top, &old, n));    /* 2. top 이 그대로면 나로 교체 */
}

static struct node *pop(void)
{
    struct node *old = atomic_load(&top);

    while (old != NULL && !atomic_compare_exchange_weak(&top, &old, old->next))
        ;                                   /* 실패하면 old 에 현재 top 이 들어온다 */
    return old;
}

static void *pusher(void *arg)
{
    long i, id = (long)arg;

    for (i = 0; i < PER; i++) {
        struct node *n = &pool[id * PER + i];

        n->value = id * PER + i + 1;
        push(n);
    }
    return NULL;
}

static void *popper(void *arg)
{
    struct node *n;
    long         sum = 0, cnt = 0;

    (void)arg;
    while ((n = pop()) != NULL) {
        sum += n->value;
        cnt++;
    }
    atomic_fetch_add(&popped_sum, sum);
    atomic_fetch_add(&popped_cnt, cnt);
    return NULL;
}

int main(void)
{
    pthread_t tid[NTHREADS];
    long      i, total = (long)NTHREADS * PER;

    pool = calloc((size_t)total, sizeof(*pool));
    if (pool == NULL)
        return EXIT_FAILURE;

    for (i = 0; i < NTHREADS; i++)
        pthread_create(&tid[i], NULL, pusher, (void *)i);
    for (i = 0; i < NTHREADS; i++)
        pthread_join(tid[i], NULL);
    for (i = 0; i < NTHREADS; i++)
        pthread_create(&tid[i], NULL, popper, NULL);
    for (i = 0; i < NTHREADS; i++)
        pthread_join(tid[i], NULL);

    printf("pushed %ld, popped %ld\n", total, atomic_load(&popped_cnt));
    printf("sum expected %ld\nsum actual   %ld\n", total * (total + 1) / 2, atomic_load(&popped_sum));
    free(pool);
    return EXIT_SUCCESS;
}
