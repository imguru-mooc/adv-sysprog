/* cow_pfn.c - fork 후 Parent 와 Child 가 같은 Physical Frame 을 공유하다가, 쓰는 순간 갈라짐을 확인한다
 * 실행: sudo ./cow_pfn
 */
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static uint64_t pfn_of(const void *addr)
{
    uint64_t entry = 0;
    int      fd    = open("/proc/self/pagemap", O_RDONLY);

    if (fd < 0) {
        perror("open pagemap");
        exit(EXIT_FAILURE);
    }
    if (pread(fd, &entry, sizeof(entry),
              (off_t)((uintptr_t)addr / 4096) * (off_t)sizeof(entry)) != (ssize_t)sizeof(entry)) {
        perror("pread");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return entry & ((1ULL << 55) - 1);
}

int main(void)
{
    char *p;
    pid_t pid;

    if (posix_memalign((void **)&p, 4096, 4096) != 0) {
        perror("posix_memalign");
        return EXIT_FAILURE;
    }
    p[0] = 'P';

    if (geteuid() != 0)
        printf("(root 가 아니면 PFN 이 0 으로 보입니다: sudo ./cow_pfn)\n");
    printf("VA %p 는 Parent 와 Child 에서 동일합니다\n", (void *)p);
    printf("[parent] fork 전        PFN 0x%" PRIx64 "\n", pfn_of(p));

    fflush(stdout);                                 /* Buffer 가 Child 로 복제되지 않도록 */

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (pid == 0) {
        printf("[child ] fork 직후      PFN 0x%" PRIx64 "   <- Parent 와 같은 Frame\n", pfn_of(p));
        p[0] = 'C';                                 /* Write -> Page Fault -> Copy */
        printf("[child ] 쓰기 후        PFN 0x%" PRIx64 "   <- 새 Frame 으로 복사됨\n", pfn_of(p));
        fflush(stdout);
        _exit(EXIT_SUCCESS);
    }
    if (waitpid(pid, NULL, 0) < 0)
        perror("waitpid");
    printf("[parent] Child 종료 후  PFN 0x%" PRIx64 "   p[0] = '%c'\n", pfn_of(p), p[0]);
    free(p);
    return EXIT_SUCCESS;
}
