/* resident.c - File 의 몇 % 가 Page Cache 에 올라와 있는지 출력한다 (fincore 와 같은 원리)
 * 사용법: ./resident <file> [file ...]
 *         ./resident -e <file>      해당 File 의 Cache 를 비운다
 */
#include "cachelib.h"

int main(int argc, char *argv[])
{
    int i, do_evict = 0;

    if (argc < 2) {
        fprintf(stderr, "사용법: %s [-e] <file> ...\n", argv[0]);
        return EXIT_FAILURE;
    }
    for (i = 1; i < argc; i++) {
        long total = 0, res;
        int  fd;

        if (strcmp(argv[i], "-e") == 0) {
            do_evict = 1;
            continue;
        }
        fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            perror(argv[i]);
            continue;
        }
        if (do_evict)
            evict(fd);
        res = resident_pages(fd, &total);
        if (res >= 0)
            printf("%-40s %8ld / %8ld pages  %5.1f%%\n", argv[i], res, total,
                   total ? 100.0 * (double)res / (double)total : 0.0);
        close(fd);
    }
    return EXIT_SUCCESS;
}
