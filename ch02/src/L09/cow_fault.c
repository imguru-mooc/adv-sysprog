/* cow_fault.c - fork 후 Child 가 쓴 Page 수만큼만 Copy-On-Write Fault 가 발생한다 */
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "faultlib.h"

#define NPAGES 10000UL
#define PAGE   4096UL

int main(void)
{
    long           mn0, mn1, mj;
    unsigned long  i;
    pid_t          pid;
    volatile char *p = mmap(NULL, NPAGES * PAGE, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    madvise((void *)p, NPAGES * PAGE, MADV_NOHUGEPAGE);
    for (i = 0; i < NPAGES; i++)            /* Parent 가 모든 Page 를 미리 채운다 */
        p[i * PAGE] = 'P';

    fflush(stdout);
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (pid == 0) {
        faults(&mn0, &mj);
        for (i = 0; i < NPAGES; i++)        /* 읽기만: 복사 없음 */
            if (p[i * PAGE] != 'P')
                _exit(EXIT_FAILURE);
        faults(&mn1, &mj);
        printf("[child] %lu Pages 읽기        : minor +%ld\n", NPAGES, mn1 - mn0);

        faults(&mn0, &mj);
        for (i = 0; i < NPAGES / 10; i++)   /* 10% 만 쓰기 */
            p[i * PAGE] = 'C';
        faults(&mn1, &mj);
        printf("[child] %lu Pages 쓰기         : minor +%ld   <- 쓴 Page 만 복사\n", NPAGES / 10, mn1 - mn0);
        fflush(stdout);
        _exit(EXIT_SUCCESS);
    }
    if (waitpid(pid, NULL, 0) < 0)
        perror("waitpid");
    printf("[parent] p[0] = '%c'  (Child 의 쓰기는 Parent 에 보이지 않는다)\n", p[0]);
    munmap((void *)p, NPAGES * PAGE);
    return EXIT_SUCCESS;
}
