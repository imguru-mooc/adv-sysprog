/* uring_raw.c - liburing 없이 System Call 만으로 io_uring 을 쓴다: 구조를 눈으로 확인하기 위한 코드
 *   io_uring_setup → mmap × 3 → SQE 작성 → io_uring_enter → CQE 읽기
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <linux/io_uring.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

static void show_maps(void)
{
    char  line[256];
    FILE *fp = fopen("/proc/self/maps", "r");

    while (fp != NULL && fgets(line, sizeof(line), fp) != NULL)
        if (strstr(line, "io_uring") != NULL)
            printf("   %s", line);
    if (fp != NULL)
        fclose(fp);
}

int main(void)
{
    struct io_uring_params p;
    struct io_uring_sqe   *sqes, *sqe;
    struct io_uring_cqe   *cqe;
    unsigned              *sq_head, *sq_tail, *sq_mask, *sq_array, *cq_head, *cq_tail, *cq_mask;
    unsigned               tail, head, idx;
    char                   buf[128] = { 0 };
    void                  *sq_ptr, *cq_ptr;
    int                    ring_fd, fd, ret;

    /* 1. io_uring_setup: Kernel 이 Ring 을 만들고 fd 를 돌려준다 */
    memset(&p, 0, sizeof(p));
    ring_fd = (int)syscall(SYS_io_uring_setup, 4, &p);
    if (ring_fd < 0) {
        perror("io_uring_setup");
        return EXIT_FAILURE;
    }
    printf("1. io_uring_setup(4) = fd %d   sq_entries=%u cq_entries=%u features=0x%x\n",
           ring_fd, p.sq_entries, p.cq_entries, p.features);

    /* 2. mmap: Kernel 이 만든 Ring 을 내 주소 공간에 연결한다 (L10). 이후 Ring 접근은 그냥 Memory 접근 */
    sq_ptr = mmap(NULL, p.sq_off.array + p.sq_entries * sizeof(unsigned), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_POPULATE, ring_fd, IORING_OFF_SQ_RING);
    cq_ptr = (p.features & IORING_FEAT_SINGLE_MMAP) ? sq_ptr :
             mmap(NULL, p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_POPULATE, ring_fd, IORING_OFF_CQ_RING);
    sqes   = mmap(NULL, p.sq_entries * sizeof(struct io_uring_sqe), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_POPULATE, ring_fd, IORING_OFF_SQES);
    if (sq_ptr == MAP_FAILED || cq_ptr == MAP_FAILED || sqes == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    printf("2. mmap 완료. /proc/self/maps:\n");
    show_maps();

    sq_head  = (unsigned *)((char *)sq_ptr + p.sq_off.head);
    sq_tail  = (unsigned *)((char *)sq_ptr + p.sq_off.tail);
    sq_mask  = (unsigned *)((char *)sq_ptr + p.sq_off.ring_mask);
    sq_array = (unsigned *)((char *)sq_ptr + p.sq_off.array);
    cq_head  = (unsigned *)((char *)cq_ptr + p.cq_off.head);
    cq_tail  = (unsigned *)((char *)cq_ptr + p.cq_off.tail);
    cq_mask  = (unsigned *)((char *)cq_ptr + p.cq_off.ring_mask);
    printf("   sizeof(SQE) = %zu Byte, sizeof(CQE) = %zu Byte\n", sizeof(struct io_uring_sqe), sizeof(struct io_uring_cqe));

    /* 3. SQE 작성: System Call 없이 공유 Memory 에 쓴다 */
    fd = open("/etc/hostname", O_RDONLY);
    tail = *sq_tail;
    idx  = tail & *sq_mask;
    sqe  = &sqes[idx];
    memset(sqe, 0, sizeof(*sqe));
    sqe->opcode    = IORING_OP_READ;
    sqe->fd        = fd;
    sqe->addr      = (unsigned long)buf;
    sqe->len       = sizeof(buf) - 1;
    sqe->off       = 0;
    sqe->user_data = 0xCAFE;
    sq_array[idx]  = idx;
    atomic_store((atomic_uint *)sq_tail, tail + 1);     /* tail 을 공개한다: Memory Barrier 포함 (L18) */
    printf("3. SQE[%u] 작성, SQ tail %u → %u  (System Call 없음)\n", idx, tail, tail + 1);

    /* 4. io_uring_enter: "SQ 에 1 개 넣었다. 1 개 완료될 때까지 기다리겠다" */
    ret = (int)syscall(SYS_io_uring_enter, ring_fd, 1, 1, IORING_ENTER_GETEVENTS, NULL, 0);
    printf("4. io_uring_enter(to_submit=1, min_complete=1) = %d   SQ head=%u tail=%u\n", ret, *sq_head, *sq_tail);

    /* 5. CQE 읽기: 역시 System Call 없이 공유 Memory 에서 읽는다 */
    head = *cq_head;
    printf("5. CQ head=%u tail=%u\n", head, atomic_load((atomic_uint *)cq_tail));
    cqe = (struct io_uring_cqe *)((char *)cq_ptr + p.cq_off.cqes) + (head & *cq_mask);
    printf("   CQE: user_data=0x%llX res=%d → %s", (unsigned long long)cqe->user_data, cqe->res, buf);
    atomic_store((atomic_uint *)cq_head, head + 1);     /* 소비했다고 알린다 */

    close(fd);
    close(ring_fd);
    return EXIT_SUCCESS;
}
