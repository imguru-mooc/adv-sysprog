/* uring_cp.c - Linked SQE: read → write 를 한 쌍으로 묶어 순서를 보장하고, 마지막에 fsync 한다
 * 사용법: ./uring_cp <src> <dst>
 */
#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BLOCK (64 * 1024)
#define PAIRS 8                         /* 한 번에 걸어 두는 (read, write) 쌍의 수 */

int main(int argc, char *argv[])
{
    static char          bufs[PAIRS][BLOCK];
    struct io_uring      ring;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    struct stat          st;
    off_t                off = 0;
    long                 enters = 0;
    int                  in, out, i, n, ret;

    if (argc < 3) {
        fprintf(stderr, "사용법: %s <src> <dst>\n", argv[0]);
        return EXIT_FAILURE;
    }
    in  = open(argv[1], O_RDONLY);
    out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (in < 0 || out < 0 || fstat(in, &st) < 0) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (io_uring_queue_init(PAIRS * 2, &ring, 0) < 0)
        return EXIT_FAILURE;

    while (off < st.st_size) {
        for (n = 0; n < PAIRS && off < st.st_size; n++, off += BLOCK) {
            unsigned len = (unsigned)(st.st_size - off < BLOCK ? st.st_size - off : BLOCK);

            sqe = io_uring_get_sqe(&ring);              /* read: 끝나야 다음 SQE 가 시작된다 */
            io_uring_prep_read(sqe, in, bufs[n], len, (__u64)off);
            sqe->flags |= IOSQE_IO_LINK;
            io_uring_sqe_set_data64(sqe, 1);

            sqe = io_uring_get_sqe(&ring);              /* write: 위 read 가 성공한 뒤에 실행된다 */
            io_uring_prep_write(sqe, out, bufs[n], len, (__u64)off);
            io_uring_sqe_set_data64(sqe, 2);
        }
        ret = io_uring_submit_and_wait(&ring, (unsigned)n * 2);
        enters++;
        if (ret < 0) {
            fprintf(stderr, "submit: %s\n", strerror(-ret));
            return EXIT_FAILURE;
        }
        for (i = 0; i < n * 2; i++) {
            io_uring_wait_cqe(&ring, &cqe);
            if (cqe->res < 0) {                         /* read 가 실패하면 연결된 write 는 -ECANCELED */
                fprintf(stderr, "%s 실패: %s\n", cqe->user_data == 1 ? "read" : "write", strerror(-cqe->res));
                return EXIT_FAILURE;
            }
            io_uring_cqe_seen(&ring, cqe);
        }
    }

    sqe = io_uring_get_sqe(&ring);                      /* fsync 도 요청 하나다 (L15) */
    io_uring_prep_fsync(sqe, out, 0);
    io_uring_submit_and_wait(&ring, 1);
    enters++;
    io_uring_wait_cqe(&ring, &cqe);
    printf("fsync res = %d\n", cqe->res);
    io_uring_cqe_seen(&ring, cqe);

    printf("%ld Byte 복사, io_uring_enter %ld 회 (read+write %ld 회 분량)\n", (long)st.st_size, enters,
           2 * (((long)st.st_size + BLOCK - 1) / BLOCK));
    io_uring_queue_exit(&ring);
    close(in);
    close(out);
    return EXIT_SUCCESS;
}
