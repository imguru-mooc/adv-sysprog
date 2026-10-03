/* logdump.c - monitor 가 mmap 으로 기록한 Ring Log 를 읽는다. monitor 가 실행 중이어도 된다 (같은 Page Cache 를 본다)
 * 사용법: ./logdump [개수=10]
 */
#include "mlog.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    struct mlog m;
    uint64_t    total, i, n = argc > 1 ? (uint64_t)atoll(argv[1]) : 10, start;

    if (mlog_open(&m, "monitor.log.bin") < 0) {
        perror("monitor.log.bin");
        return EXIT_FAILURE;
    }
    total = m.hdr->total;
    start = total > n ? total - n : 0;
    if (total - start > MLOG_CAP)
        start = total - MLOG_CAP;
    printf("기록 %llu 건 (용량 %u), 마지막 %llu 건:\n", (unsigned long long)total, m.hdr->cap, (unsigned long long)(total - start));
    for (i = start; i < total; i++) {
        struct mrec *r = &m.rec[i % MLOG_CAP];
        time_t       t = (time_t)(r->ts_ms / 1000);
        char         ts[16];

        strftime(ts, sizeof(ts), "%H:%M:%S", localtime(&t));
        if (r->kind == 'E')
            printf("%s.%03d  EVENT   %s\n", ts, (int)(r->ts_ms % 1000), r->comm);
        else
            printf("%s.%03d  %-15s pid %-6d %c cpu %5.1f%% rss %7d kB thr %2u fds %3d\n", ts, (int)(r->ts_ms % 1000),
                   r->comm, r->pid, r->state, r->cpu_x10 / 10.0, r->rss_kb, r->threads, r->fds);
    }
    munmap(m.hdr, m.bytes);
    close(m.fd);
    return EXIT_SUCCESS;
}
