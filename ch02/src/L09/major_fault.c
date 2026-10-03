/* major_fault.c - File 을 mmap 해서 읽는다. Page Cache 에 없으면 Disk I/O 가 필요한 Major Fault 가 된다
 * 사용법: ./major_fault make      (128MB File big.dat 생성)
 *         ./major_fault           (mmap 으로 전부 읽고 Fault 횟수와 시간 출력)
 * Cache 비우기: sync; echo 3 | sudo tee /proc/sys/vm/drop_caches
 */
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "faultlib.h"

#define FILE_NAME "big.dat"
#define FILE_SIZE (128UL * 1024 * 1024)

static int make_file(void)
{
    static char buf[1 << 20];
    size_t      done;
    int         fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }
    memset(buf, 'x', sizeof(buf));
    for (done = 0; done < FILE_SIZE; done += sizeof(buf))
        if (write(fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf)) {
            perror("write");
            return EXIT_FAILURE;
        }
    if (fsync(fd) < 0)
        perror("fsync");
    close(fd);
    printf("%s (%lu MB) 생성\n", FILE_NAME, FILE_SIZE >> 20);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    struct timespec t0, t1;
    struct stat     st;
    long            mn0, mj0, mn1, mj1;
    unsigned long   i, sum = 0;
    volatile char  *p;
    int             fd;

    if (argc > 1 && strcmp(argv[1], "make") == 0)
        return make_file();

    fd = open(FILE_NAME, O_RDONLY);
    if (fd < 0 || fstat(fd, &st) < 0) {
        perror(FILE_NAME " (먼저 ./major_fault make)");
        return EXIT_FAILURE;
    }
    p = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    close(fd);                              /* mmap 후에는 fd 를 닫아도 Mapping 은 유지된다 */

    faults(&mn0, &mj0);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < (unsigned long)st.st_size; i += 4096)
        sum += p[i];
    clock_gettime(CLOCK_MONOTONIC, &t1);
    faults(&mn1, &mj1);

    printf("minor +%ld   major +%ld   %.1f ms   (sum=%lu)\n", mn1 - mn0, mj1 - mj0,
           (double)(t1.tv_sec - t0.tv_sec) * 1e3 + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6, sum);
    munmap((void *)p, (size_t)st.st_size);
    return EXIT_SUCCESS;
}
