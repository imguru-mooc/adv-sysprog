/* fault_count.c - Page 를 처음 건드릴 때마다 Page Fault 가 한 번씩 발생함을 센다 */
#include <sys/mman.h>
#include "faultlib.h"

#define NPAGES 10000UL
#define PAGE   4096UL

int main(void)
{
    long           mn0, mj0, mn1, mj1, mn2, mj2, mn3, mj3;
    unsigned long  i;
    volatile char *p;

    faults(&mn0, &mj0);
    p = mmap(NULL, NPAGES * PAGE, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    madvise((void *)p, NPAGES * PAGE, MADV_NOHUGEPAGE);
    faults(&mn1, &mj1);

    for (i = 0; i < NPAGES; i++)            /* 첫 번째 접근 */
        p[i * PAGE] = 1;
    faults(&mn2, &mj2);

    for (i = 0; i < NPAGES; i++)            /* 두 번째 접근 */
        p[i * PAGE] = 2;
    faults(&mn3, &mj3);

    printf("Page 수                 : %lu\n", NPAGES);
    printf("mmap() 호출             : minor +%ld\n", mn1 - mn0);
    printf("첫 번째 쓰기 (10000 회) : minor +%ld  major +%ld\n", mn2 - mn1, mj2 - mj1);
    printf("두 번째 쓰기 (10000 회) : minor +%ld  major +%ld\n", mn3 - mn2, mj3 - mj2);
    munmap((void *)p, NPAGES * PAGE);
    return EXIT_SUCCESS;
}
