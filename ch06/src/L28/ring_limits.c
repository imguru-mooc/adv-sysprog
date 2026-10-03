/* ring_limits.c - 오류는 errno 가 아니라 CQE 의 res 로 온다 / SQ 가 가득 차면 get_sqe 는 NULL */
#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    struct io_uring      ring;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    char                 buf[16];
    int                  i, n, fd = open(".", O_RDONLY | O_DIRECTORY);      /* Directory: read 할 수 없는 대상 */

    if (io_uring_queue_init(4, &ring, 0) < 0)
        return EXIT_FAILURE;

    /* 1. 실패하는 요청 두 개 */
    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_read(sqe, 9999, buf, sizeof(buf), 0);         /* 존재하지 않는 fd */
    io_uring_sqe_set_data64(sqe, 1);
    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_read(sqe, fd, buf, sizeof(buf), 0);           /* Directory 를 read */
    io_uring_sqe_set_data64(sqe, 2);
    n = io_uring_submit(&ring);
    printf("submit = %d  ← 제출 자체는 성공한다\n", n);
    for (i = 0; i < 2; i++) {
        io_uring_wait_cqe(&ring, &cqe);
        printf("   요청 %llu: res = %d → %s\n", (unsigned long long)cqe->user_data, cqe->res, strerror(-cqe->res));
        io_uring_cqe_seen(&ring, cqe);
    }

    /* 2. SQ 를 가득 채운다 */
    for (n = 0; (sqe = io_uring_get_sqe(&ring)) != NULL; n++)
        io_uring_prep_nop(sqe);
    printf("get_sqe 가 NULL 을 return 하기 전까지 얻은 SQE: %d 개 (SQ 크기 %u)\n", n, ring.sq.ring_entries);
    printf("   → submit 해서 Kernel 이 가져가게 하면 다시 비워진다: submit = %d\n", io_uring_submit(&ring));
    sqe = io_uring_get_sqe(&ring);
    printf("   → submit 후 get_sqe = %s\n", sqe != NULL ? "성공" : "NULL");

    io_uring_queue_exit(&ring);
    if (fd >= 0)
        close(fd);
    return EXIT_SUCCESS;
}
