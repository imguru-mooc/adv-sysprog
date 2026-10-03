/* batch_compare.c - 같은 File 을 4KB 단위로 읽는다: read() Loop 대 io_uring Batch
 * Page Cache 에 올라와 있는 상태에서 "System Call 횟수" 의 효과만 본다
 * 사용법: ./batch_compare [batch 크기 ...]     기본: 1 8 64 256
 */
#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NAME   "batch.dat"
#define MB     64
#define BLOCK  4096
#define NBLK   (MB * 1024L * 1024L / BLOCK)

static char *data;

static double now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

static void prepare(void)
{
    static char buf[1 << 20];
    int         i, fd;

    if (access(NAME, F_OK) == 0)
        return;
    fd = open(NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    memset(buf, 'b', sizeof(buf));
    for (i = 0; i < MB; i++)
        if (write(fd, buf, sizeof(buf)) < 0)
            exit(EXIT_FAILURE);
    close(fd);
}

static void run_read(int fd)
{
    double t0 = now_ms();
    long   i;

    for (i = 0; i < NBLK; i++)
        if (pread(fd, data + i * BLOCK, BLOCK, i * BLOCK) != BLOCK)
            exit(EXIT_FAILURE);
    printf("%-22s %8ld syscalls %9.1f ms\n", "pread() Loop", NBLK, now_ms() - t0);
}

static void run_uring(int fd, unsigned batch)
{
    struct io_uring      ring;
    struct io_uring_cqe *cqe;
    char                 tag[32];
    double               t0;
    long                 next = 0, done = 0, enters = 0;
    unsigned             k, n;

    if (io_uring_queue_init(batch, &ring, 0) < 0)
        exit(EXIT_FAILURE);
    t0 = now_ms();
    while (done < NBLK) {
        for (n = 0; n < batch && next < NBLK; n++, next++)
            io_uring_prep_read(io_uring_get_sqe(&ring), fd, data + next * BLOCK, BLOCK, (__u64)(next * BLOCK));
        io_uring_submit_and_wait(&ring, n);             /* n 개 제출 + n 개 완료 대기: System Call 1 회 */
        enters++;
        for (k = 0; k < n; k++) {
            io_uring_peek_cqe(&ring, &cqe);
            if (cqe->res != BLOCK)
                exit(EXIT_FAILURE);
            io_uring_cqe_seen(&ring, cqe);
            done++;
        }
    }
    snprintf(tag, sizeof(tag), "io_uring batch %u", batch);
    printf("%-22s %8ld syscalls %9.1f ms\n", tag, enters, now_ms() - t0);
    io_uring_queue_exit(&ring);
}

int main(int argc, char *argv[])
{
    unsigned defaults[] = { 1, 8, 64, 256 };
    int      i, fd;

    prepare();
    fd = open(NAME, O_RDONLY);
    data = malloc((size_t)MB << 20);
    if (fd < 0 || data == NULL)
        return EXIT_FAILURE;
    memset(data, 0, (size_t)MB << 20);                  /* Page Fault 를 측정 밖으로 */
    run_read(fd);                                       /* 첫 실행은 준비 운동 (Cache 채우기) */

    printf("\n%dMB 를 %dKB 씩 %ld 번 읽기 (모두 Page Cache Hit)\n", MB, BLOCK / 1024, NBLK);
    run_read(fd);
    if (argc > 1)
        for (i = 1; i < argc; i++)
            run_uring(fd, (unsigned)atoi(argv[i]));
    else
        for (i = 0; i < 4; i++)
            run_uring(fd, defaults[i]);
    return EXIT_SUCCESS;
}
