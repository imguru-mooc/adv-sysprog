/* uring_probe.c - 이 Kernel 의 io_uring 이 무엇을 지원하는지 확인한다 */
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct { int op; const char *name; } ops[] = {
    { IORING_OP_NOP, "NOP" }, { IORING_OP_READV, "READV" }, { IORING_OP_WRITEV, "WRITEV" },
    { IORING_OP_FSYNC, "FSYNC" }, { IORING_OP_READ_FIXED, "READ_FIXED" }, { IORING_OP_WRITE_FIXED, "WRITE_FIXED" },
    { IORING_OP_POLL_ADD, "POLL_ADD" }, { IORING_OP_TIMEOUT, "TIMEOUT" }, { IORING_OP_OPENAT, "OPENAT" },
    { IORING_OP_CLOSE, "CLOSE" }, { IORING_OP_STATX, "STATX" }, { IORING_OP_READ, "READ" },
    { IORING_OP_WRITE, "WRITE" }, { IORING_OP_FADVISE, "FADVISE" }, { IORING_OP_SPLICE, "SPLICE" },
    { IORING_OP_RENAMEAT, "RENAMEAT" }, { IORING_OP_UNLINKAT, "UNLINKAT" }, { IORING_OP_LINK_TIMEOUT, "LINK_TIMEOUT" },
};

int main(void)
{
    struct io_uring_probe *probe;
    struct io_uring        ring;
    FILE                  *fp;
    size_t                 i;
    int                    ret, v, n = 0;

    fp = fopen("/proc/sys/kernel/io_uring_disabled", "r");
    if (fp != NULL && fscanf(fp, "%d", &v) == 1)
        printf("kernel.io_uring_disabled = %d  (0: 허용, 1: 권한 있는 Process 만, 2: 전부 금지)\n", v);
    if (fp != NULL)
        fclose(fp);

    ret = io_uring_queue_init(8, &ring, 0);
    if (ret < 0) {
        fprintf(stderr, "io_uring_queue_init: %s\n", strerror(-ret));
        fprintf(stderr, "  ENOSYS: Kernel 미지원 / EPERM: sysctl 또는 seccomp 로 차단 (Container 에서 흔함)\n");
        return EXIT_FAILURE;
    }
    printf("features = 0x%x%s%s%s\n", ring.features,
           (ring.features & IORING_FEAT_SINGLE_MMAP) ? "  SINGLE_MMAP" : "",
           (ring.features & IORING_FEAT_NODROP) ? "  NODROP" : "",
           (ring.features & IORING_FEAT_FAST_POLL) ? "  FAST_POLL" : "");

    probe = io_uring_get_probe_ring(&ring);
    if (probe == NULL)
        return EXIT_FAILURE;
    printf("지원하는 마지막 opcode 번호 = %d\n", probe->last_op);
    for (i = 0; i < sizeof(ops) / sizeof(ops[0]); i++)
        if (io_uring_opcode_supported(probe, ops[i].op)) {
            printf("%-13s", ops[i].name);
            if (++n % 6 == 0)
                putchar('\n');
        }
    putchar('\n');
    io_uring_free_probe(probe);
    io_uring_queue_exit(&ring);
    return EXIT_SUCCESS;
}
