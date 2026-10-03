/* pagemap.c - /proc/self/pagemap 으로 Virtual Page 가 어느 Physical Frame 에 있는지 확인한다
 * PFN 을 보려면 root 권한이 필요하다:  sudo ./pagemap
 */
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define PM_PRESENT (1ULL << 63)
#define PM_SWAPPED (1ULL << 62)
#define PM_PFN     ((1ULL << 55) - 1)

static void lookup(const char *tag, const void *addr)
{
    uint64_t entry;
    long     psz = sysconf(_SC_PAGESIZE);
    off_t    off = (off_t)((uintptr_t)addr / (uintptr_t)psz) * (off_t)sizeof(entry);
    int      fd  = open("/proc/self/pagemap", O_RDONLY);

    if (fd < 0) {
        perror("open pagemap");
        exit(EXIT_FAILURE);
    }
    if (pread(fd, &entry, sizeof(entry), off) != (ssize_t)sizeof(entry)) {
        perror("pread");
        exit(EXIT_FAILURE);
    }
    close(fd);

    printf("%-22s VA %p  present=%d", tag, addr, (entry & PM_PRESENT) ? 1 : 0);
    if (entry & PM_PRESENT) {
        uint64_t pfn = entry & PM_PFN;

        if (pfn != 0)
            printf("  PFN 0x%" PRIx64 "  PA 0x%" PRIx64,
                   pfn, pfn * (uint64_t)psz + (uintptr_t)addr % (uintptr_t)psz);
        else
            printf("  PFN (root 권한 필요)");
    }
    printf("\n");
}

int main(void)
{
    char *p = mmap(NULL, 3 * 4096, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    lookup("page0 (건드리기 전)", p);
    p[0] = 'A';                         /* 첫 번째 Page 에만 쓴다 */
    lookup("page0 (쓴 뒤)", p);
    lookup("page1 (건드리지 않음)", p + 4096);
    lookup("main 함수의 코드", (void *)(uintptr_t)main);

    munmap(p, 3 * 4096);
    return EXIT_SUCCESS;
}
