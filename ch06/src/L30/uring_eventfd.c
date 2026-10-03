/* uring_eventfd.c - io_uring 의 완료를 eventfd 로 받아 Chapter 5 의 epoll Event Loop 에 통합한다
 *   10ms timerfd 심장 박동 + 256MB File 읽기(io_uring). L26 의 loop_stall 과 비교한다
 * 사용법: ./uring_eventfd           Buffered I/O (Cache Miss 는 Kernel 의 io-wq Worker Thread 가 처리)
 *         ./uring_eventfd direct    O_DIRECT (Worker 없이 Block Layer 에 직접 비동기 제출)
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <liburing.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#define NAME  "big.dat"
#define MB    256
#define CHUNK (1 << 20)

enum { SRC_TIMER, SRC_URING };

static char *data;

static double now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

static void prepare(void)
{
    static char buf[CHUNK];
    int         i, fd;

    if (access(NAME, F_OK) != 0) {
        printf("%s (%dMB) 생성 중...\n", NAME, MB);
        fd = open(NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        memset(buf, 'x', sizeof(buf));
        for (i = 0; i < MB; i++)
            if (write(fd, buf, sizeof(buf)) < 0)
                exit(EXIT_FAILURE);
        fsync(fd);
        close(fd);
    }
    fd = open(NAME, O_RDONLY);
    fsync(fd);
    posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
    close(fd);
}

int main(int argc, char *argv[])
{
    struct itimerspec    its = { { 0, 10 * 1000 * 1000 }, { 0, 10 * 1000 * 1000 } };
    struct epoll_event   ev, events[4];
    struct io_uring      ring;
    struct io_uring_cqe *cqe;
    uint64_t             v, missed = 0;
    double               t0, t_req = 0;
    unsigned             workers[2] = { 2, 0 };
    int                  direct = (argc > 1 && strcmp(argv[1], "direct") == 0);
    int                  epfd, tfd, efd, fd, i, n, tick = 0, pending = 0, submitted = 0;

    prepare();
    if (posix_memalign((void **)&data, 4096, (size_t)MB << 20) != 0)   /* O_DIRECT 를 위해 정렬 (L14) */
        return EXIT_FAILURE;
    memset(data, 0, (size_t)MB << 20);                  /* Page Fault 를 미리 끝내 둔다 (L09) */
    fd   = open(NAME, O_RDONLY | (direct ? O_DIRECT : 0));
    epfd = epoll_create1(0);
    tfd  = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    efd  = eventfd(0, EFD_NONBLOCK);
    if (fd < 0 || io_uring_queue_init(MB, &ring, 0) < 0)
        return EXIT_FAILURE;
    io_uring_register_iowq_max_workers(&ring, workers);  /* io-wq Worker 를 2 개로 제한 (CPU 가 적은 VM 배려) */
    printf("Mode: %s\n", direct ? "O_DIRECT" : "Buffered");
    io_uring_register_eventfd(&ring, efd);              /* CQE 가 생길 때마다 Kernel 이 이 eventfd 에 write 한다 */

    ev.events = EPOLLIN; ev.data.u32 = SRC_TIMER; epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);
    ev.events = EPOLLIN; ev.data.u32 = SRC_URING; epoll_ctl(epfd, EPOLL_CTL_ADD, efd, &ev);
    timerfd_settime(tfd, 0, &its, NULL);
    t0 = now_ms();

    while (tick < 3 || pending > 0 || !submitted) {
        n = epoll_wait(epfd, events, 4, -1);
        for (i = 0; i < n; i++) {
            if (events[i].data.u32 == SRC_TIMER) {
                if (read(tfd, &v, sizeof(v)) != sizeof(v))
                    continue;
                tick++;
                if (v > 1)
                    missed += v - 1;
                if (tick <= 3 || v > 1)
                    printf("[%7.1f ms] tick %d (만료 횟수 %llu)\n", now_ms() - t0, tick, (unsigned long long)v);
                if (tick == 3) {                        /* Handler: 읽기를 "요청만" 하고 바로 돌아간다 */
                    int k;

                    for (k = 0; k < MB; k++)
                        io_uring_prep_read(io_uring_get_sqe(&ring), fd, data + (size_t)k * CHUNK, CHUNK,
                                           (__u64)k * CHUNK);
                    t_req = now_ms();
                    pending = io_uring_submit(&ring);
                    submitted = 1;
                    printf("             Handler: read 요청 %d 개 제출에 %.2f ms → Loop 로 복귀\n", pending,
                           now_ms() - t_req);
                }
            } else {                                    /* io_uring 완료 통지 */
                if (read(efd, &v, sizeof(v)) != sizeof(v))
                    continue;
                while (io_uring_peek_cqe(&ring, &cqe) == 0) {
                    if (cqe->res < 0)
                        fprintf(stderr, "read: %s\n", strerror(-cqe->res));
                    io_uring_cqe_seen(&ring, cqe);
                    pending--;
                }
                if (pending == 0)
                    printf("[%7.1f ms] %dMB 읽기 완료 (요청 후 %.1f ms). 그동안 tick 은 %d 까지 진행\n",
                           now_ms() - t0, MB, now_ms() - t_req, tick);
            }
        }
    }
    printf("놓친 Tick: %llu\n", (unsigned long long)missed);
    io_uring_queue_exit(&ring);
    return EXIT_SUCCESS;
}
