/* out_of_order.c - 완료 순서는 제출 순서와 다르다. 그래서 user_data 가 필요하다
 *   TIMEOUT 요청 3 개를 300ms, 100ms, 200ms 순서로 제출한다
 */
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct request {                        /* 요청마다 하나씩. 포인터를 user_data 에 넣는다 */
    const char              *name;
    struct __kernel_timespec ts;
};

static double now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

int main(void)
{
    struct request       reqs[3] = { { "A (300ms)", { 0, 300000000 } }, { "B (100ms)", { 0, 100000000 } },
                                     { "C (200ms)", { 0, 200000000 } } };
    struct io_uring      ring;
    struct io_uring_cqe *cqe;
    double               t0;
    int                  i;

    if (io_uring_queue_init(8, &ring, 0) < 0)
        return EXIT_FAILURE;

    for (i = 0; i < 3; i++) {
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);

        io_uring_prep_timeout(sqe, &reqs[i].ts, 0, 0);
        io_uring_sqe_set_data(sqe, &reqs[i]);           /* user_data = 구조체 포인터 */
        printf("제출: %s\n", reqs[i].name);
    }
    t0 = now_ms();
    io_uring_submit(&ring);                             /* System Call 한 번에 3 개 */

    for (i = 0; i < 3; i++) {
        struct request *r;

        io_uring_wait_cqe(&ring, &cqe);
        r = io_uring_cqe_get_data(cqe);
        printf("[%6.1f ms] 완료: %s  res=%d (%s)\n", now_ms() - t0, r->name, cqe->res, strerror(-cqe->res));
        io_uring_cqe_seen(&ring, cqe);
    }
    io_uring_queue_exit(&ring);
    return EXIT_SUCCESS;
}
