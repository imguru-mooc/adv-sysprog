/* uring_sqpoll.c - IORING_SETUP_SQPOLL: Kernel Thread 가 SQ 를 계속 들여다본다 → 제출에 System Call 이 필요 없다
 * 실행 중에 다른 Terminal 에서: ps -eLo pid,tid,comm | grep iou-sqp
 */
#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ROUNDS 20000

int main(void)
{
    struct io_uring_params p;
    struct io_uring        ring;
    struct io_uring_cqe   *cqe;
    char                   buf[64];
    int                    fd = open("/etc/hostname", O_RDONLY), ret, i;
    long                   need_wakeup = 0;

    memset(&p, 0, sizeof(p));
    p.flags          = IORING_SETUP_SQPOLL;
    p.sq_thread_idle = 2000;                            /* 2 초 동안 일이 없으면 Kernel Thread 가 잠든다 */
    ret = io_uring_queue_init_params(8, &ring, &p);
    if (ret < 0) {
        fprintf(stderr, "SQPOLL 설정 실패: %s (오래된 Kernel 에서는 root 권한 필요)\n", strerror(-ret));
        return EXIT_FAILURE;
    }
    printf("SQPOLL Ring 생성. pid %d 의 Kernel Thread: iou-sqp-%d\n", (int)getpid(), (int)getpid());

    for (i = 0; i < ROUNDS; i++) {
        io_uring_prep_read(io_uring_get_sqe(&ring), fd, buf, sizeof(buf), 0);
        if (IO_URING_READ_ONCE(*ring.sq.kflags) & IORING_SQ_NEED_WAKEUP)
            need_wakeup++;                              /* Kernel Thread 가 자고 있을 때만 io_uring_enter 가 필요 */
        io_uring_submit(&ring);                         /* SQPOLL 에서는 보통 tail 만 갱신하고 끝난다 */
        while (io_uring_peek_cqe(&ring, &cqe) != 0)     /* 완료도 System Call 없이 공유 Memory 에서 확인 (Busy Poll) */
            ;
        if (cqe->res < 0) {
            fprintf(stderr, "read: %s\n", strerror(-cqe->res));
            return EXIT_FAILURE;
        }
        io_uring_cqe_seen(&ring, cqe);
    }
    printf("read %d 회 완료. 그중 Kernel Thread 를 깨우려고 io_uring_enter 가 필요했던 횟수: %ld\n", ROUNDS, need_wakeup);
    printf("strace -c 로 실행해 io_uring_enter 횟수를 확인해 보세요.\n");
    io_uring_queue_exit(&ring);
    return EXIT_SUCCESS;
}
