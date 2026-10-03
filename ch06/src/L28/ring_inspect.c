/* ring_inspect.c - 요청 3 개가 지나가는 동안 SQ 와 CQ 의 head / tail 이 어떻게 움직이는지 출력한다 */
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void show(struct io_uring *r, const char *tag)
{
    printf("%-34s SQ head=%u tail=%u (local tail=%u) | CQ head=%u tail=%u\n", tag,
           *r->sq.khead, *r->sq.ktail, r->sq.sqe_tail, *r->cq.khead, *r->cq.ktail);
}

int main(void)
{
    struct io_uring      ring;
    struct io_uring_cqe *cqe;
    int                  i, ret;

    ret = io_uring_queue_init(4, &ring, 0);
    if (ret < 0) {
        fprintf(stderr, "io_uring_queue_init: %s\n", strerror(-ret));
        return EXIT_FAILURE;
    }
    printf("SQ %u 칸 (mask 0x%x), CQ %u 칸 (mask 0x%x)\n\n", ring.sq.ring_entries, ring.sq.ring_mask,
           ring.cq.ring_entries, ring.cq.ring_mask);
    show(&ring, "0. 초기 상태");

    for (i = 0; i < 3; i++) {
        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);     /* User 쪽 local tail 만 전진 */

        io_uring_prep_nop(sqe);                                 /* 아무 일도 하지 않는 요청 */
        io_uring_sqe_set_data64(sqe, 100 + (unsigned)i);
    }
    show(&ring, "1. get_sqe × 3 (아직 비공개)");

    io_uring_submit(&ring);                                     /* tail 공개 + io_uring_enter */
    show(&ring, "2. submit 후");

    for (i = 0; i < 3; i++) {
        io_uring_wait_cqe(&ring, &cqe);
        printf("   CQE user_data=%llu res=%d\n", (unsigned long long)cqe->user_data, cqe->res);
        io_uring_cqe_seen(&ring, cqe);                          /* CQ head 전진 */
    }
    show(&ring, "3. CQE 3 개 소비 후");

    for (i = 0; i < 3; i++)                                     /* 한 바퀴 더: index 가 mask 로 감긴다 */
        io_uring_prep_nop(io_uring_get_sqe(&ring));
    io_uring_submit_and_wait(&ring, 3);
    show(&ring, "4. 3 개 더 제출하고 완료 대기");
    printf("   → tail=6 이지만 실제 칸은 6 & 0x3 = %u. head 와 tail 은 계속 증가하고 mask 로 위치를 구한다\n", 6 & ring.sq.ring_mask);

    io_uring_queue_exit(&ring);
    return EXIT_SUCCESS;
}
