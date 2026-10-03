/* uring_fixed.c - 미리 등록해 두기: Registered Buffer 와 Registered File
 * 요청마다 반복되는 준비 작업(Buffer Page 고정, fd → struct file 조회)을 한 번만 한다
 */
#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#define NAME   "fixed.dat"
#define BLOCK  4096
#define NBLK   4096                     /* 16MB */
#define ROUNDS 20

static double now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

static void run(struct io_uring *ring, int fd, char *buf, int fixed)
{
    struct io_uring_cqe *cqe;
    double               t0 = now_ms();
    int                  r, i, k, n;

    for (r = 0; r < ROUNDS; r++)
        for (i = 0; i < NBLK; i += 64) {
            for (n = 0; n < 64; n++) {
                struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

                if (fixed) {
                    io_uring_prep_read_fixed(sqe, 0, buf + n * BLOCK, BLOCK, (__u64)(i + n) * BLOCK, 0);
                    sqe->flags |= IOSQE_FIXED_FILE;     /* fd 자리의 0 은 "등록된 File Table 의 0 번" */
                } else {
                    io_uring_prep_read(sqe, fd, buf + n * BLOCK, BLOCK, (__u64)(i + n) * BLOCK);
                }
            }
            io_uring_submit_and_wait(ring, 64);
            for (k = 0; k < 64; k++) {
                io_uring_peek_cqe(ring, &cqe);
                if (cqe->res != BLOCK) {
                    fprintf(stderr, "res = %d (%s)\n", cqe->res, cqe->res < 0 ? strerror(-cqe->res) : "short");
                    exit(EXIT_FAILURE);
                }
                io_uring_cqe_seen(ring, cqe);
            }
        }
    printf("%-34s %8.1f ms  (read %d 회)\n", fixed ? "READ_FIXED + IOSQE_FIXED_FILE" : "일반 READ", now_ms() - t0,
           ROUNDS * NBLK);
}

int main(void)
{
    struct io_uring ring;
    struct iovec    iov;
    char           *buf;
    int             fd, i, ret;

    fd = open(NAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0 || posix_memalign((void **)&buf, 4096, 64 * BLOCK) != 0)
        return EXIT_FAILURE;
    memset(buf, 'f', 64 * BLOCK);
    for (i = 0; i < NBLK / 64; i++)
        if (write(fd, buf, 64 * BLOCK) < 0)
            return EXIT_FAILURE;

    if (io_uring_queue_init(64, &ring, 0) < 0)
        return EXIT_FAILURE;
    run(&ring, fd, buf, 0);

    iov.iov_base = buf;
    iov.iov_len  = 64 * BLOCK;
    ret = io_uring_register_buffers(&ring, &iov, 1);    /* Buffer 의 Page 를 미리 고정(pin)하고 Mapping 해 둔다 */
    if (ret < 0) {
        fprintf(stderr, "register_buffers: %s (ulimit -l 의 Locked Memory 한도를 확인)\n", strerror(-ret));
        return EXIT_FAILURE;
    }
    ret = io_uring_register_files(&ring, &fd, 1);       /* fd → struct file 참조를 미리 잡아 둔다 */
    if (ret < 0) {
        fprintf(stderr, "register_files: %s\n", strerror(-ret));
        return EXIT_FAILURE;
    }
    run(&ring, fd, buf, 1);

    io_uring_queue_exit(&ring);
    unlink(NAME);
    return EXIT_SUCCESS;
}
