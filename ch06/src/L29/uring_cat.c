/* uring_cat.c - File 전체를 Block 단위 요청 여러 개로 나눠 한 번에 제출하고, 완료된 순서대로 받아 조립한다
 * 사용법: ./uring_cat <file>
 */
#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BLOCK 4096
#define QD    64                        /* Queue Depth: 동시에 걸어 둘 수 있는 요청 수 */

struct req {
    off_t  off;
    size_t len;
};

int main(int argc, char *argv[])
{
    struct io_uring      ring;
    struct io_uring_cqe *cqe;
    struct stat          st;
    struct req          *reqs;
    char                *data;
    long                 nblocks, i, submitted = 0, completed = 0, inflight = 0, enters = 0;
    int                  fd, ret;

    if (argc < 2) {
        fprintf(stderr, "사용법: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    fd = open(argv[1], O_RDONLY);
    if (fd < 0 || fstat(fd, &st) < 0) {
        perror(argv[1]);
        return EXIT_FAILURE;
    }
    nblocks = (st.st_size + BLOCK - 1) / BLOCK;
    data = malloc((size_t)st.st_size + 1);
    reqs = calloc((size_t)nblocks, sizeof(*reqs));
    if (data == NULL || reqs == NULL || io_uring_queue_init(QD, &ring, 0) < 0)
        return EXIT_FAILURE;

    while (completed < nblocks) {
        /* 1. SQ 에 자리가 있고 보낼 것이 남아 있는 동안 SQE 를 채운다 */
        while (submitted < nblocks && inflight < QD) {
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);

            if (sqe == NULL)
                break;
            i = submitted;
            reqs[i].off = (off_t)i * BLOCK;
            reqs[i].len = (size_t)(st.st_size - reqs[i].off < BLOCK ? st.st_size - reqs[i].off : BLOCK);
            io_uring_prep_read(sqe, fd, data + reqs[i].off, (unsigned)reqs[i].len, (__u64)reqs[i].off);
            io_uring_sqe_set_data(sqe, &reqs[i]);
            submitted++;
            inflight++;
        }
        /* 2. 한 번의 System Call 로 전부 제출하고, 적어도 1 개가 끝날 때까지 기다린다 */
        ret = io_uring_submit_and_wait(&ring, 1);
        enters++;
        if (ret < 0) {
            fprintf(stderr, "submit: %s\n", strerror(-ret));
            return EXIT_FAILURE;
        }
        /* 3. 도착해 있는 CQE 를 전부 거둔다 (System Call 없음) */
        while (io_uring_peek_cqe(&ring, &cqe) == 0) {
            struct req *r = io_uring_cqe_get_data(cqe);

            if (cqe->res < 0) {
                fprintf(stderr, "read @%ld: %s\n", (long)r->off, strerror(-cqe->res));
                return EXIT_FAILURE;
            }
            if ((size_t)cqe->res != r->len)             /* Short Read: 실전에서는 남은 부분을 다시 제출한다 */
                fprintf(stderr, "short read @%ld: %d / %zu\n", (long)r->off, cqe->res, r->len);
            io_uring_cqe_seen(&ring, cqe);
            completed++;
            inflight--;
        }
    }
    fwrite(data, 1, (size_t)st.st_size, stdout);
    fprintf(stderr, "\n[uring_cat] %ld Byte, read 요청 %ld 개, io_uring_enter %ld 회\n", (long)st.st_size, nblocks, enters);

    io_uring_queue_exit(&ring);
    free(data);
    free(reqs);
    close(fd);
    return EXIT_SUCCESS;
}
