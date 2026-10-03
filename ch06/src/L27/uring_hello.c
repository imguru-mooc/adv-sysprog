/* uring_hello.c - liburing 으로 File 을 한 번 읽는 가장 작은 io_uring 프로그램
 * 사용법: ./uring_hello [file]
 */
#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    const char          *path = argc > 1 ? argv[1] : "/etc/hostname";
    struct io_uring      ring;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    char                 buf[256] = { 0 };
    int                  fd, ret;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror(path);
        return EXIT_FAILURE;
    }

    ret = io_uring_queue_init(8, &ring, 0);             /* 1. Ring 생성: SQ 8 칸 (CQ 는 16 칸) */
    if (ret < 0) {                                      /*    liburing 은 errno 가 아니라 -errno 를 return 한다 */
        fprintf(stderr, "io_uring_queue_init: %s\n", strerror(-ret));
        return EXIT_FAILURE;
    }

    sqe = io_uring_get_sqe(&ring);                      /* 2. 빈 SQE 하나를 얻는다 (System Call 아님) */
    io_uring_prep_read(sqe, fd, buf, sizeof(buf) - 1, 0);   /* 3. "fd 의 offset 0 에서 buf 로 읽어라" 를 적는다 */
    io_uring_sqe_set_data64(sqe, 42);                   /*    완료 때 그대로 돌려받을 꼬리표 */

    ret = io_uring_submit(&ring);                       /* 4. 제출: io_uring_enter() System Call */
    printf("io_uring_submit = %d  (제출된 SQE 수)\n", ret);

    ret = io_uring_wait_cqe(&ring, &cqe);               /* 5. 완료를 기다린다 */
    if (ret < 0) {
        fprintf(stderr, "io_uring_wait_cqe: %s\n", strerror(-ret));
        return EXIT_FAILURE;
    }
    printf("CQE: user_data = %llu, res = %d\n", (unsigned long long)cqe->user_data, cqe->res);
    if (cqe->res < 0)                                   /*    res 는 read() 의 return 값. 실패면 -errno */
        fprintf(stderr, "read 실패: %s\n", strerror(-cqe->res));
    else
        printf("내용: %s", buf);
    io_uring_cqe_seen(&ring, cqe);                      /* 6. CQE 를 소비했다고 표시 (CQ head 전진) */

    io_uring_queue_exit(&ring);
    close(fd);
    return EXIT_SUCCESS;
}
