/* procstat.h - /proc/PID 에서 한 시점의 Sample 을 읽는다 (L31 의 procmon 을 Library 로 정리한 것) */
#ifndef PROCSTAT_H
#define PROCSTAT_H

#include <dirent.h>
#include <stdio.h>
#include <string.h>

struct psample {
    int           pid;
    char          comm[16];
    char          state;
    unsigned long cpu_ticks;            /* utime + stime (누적) */
    long          rss_kb, threads;
    int           fds;
};

static inline int ps_read(int pid, struct psample *s, int with_fds)
{
    char  path[64], buf[1024], *l, *r;
    unsigned long ut = 0, st = 0;
    long  rss_pages = 0;
    FILE *fp;

    memset(s, 0, sizeof(*s));
    s->pid = pid;
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fp = fopen(path, "r");
    if (fp == NULL)
        return -1;                                      /* 읽는 사이에 Process 가 사라지는 것은 정상 상황이다 */
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    l = strchr(buf, '(');
    r = strrchr(buf, ')');                              /* comm 에 공백과 괄호가 있을 수 있다: 마지막 ')' 기준 */
    if (l == NULL || r == NULL || r < l)
        return -1;
    snprintf(s->comm, sizeof(s->comm), "%.*s", (int)(r - l - 1 > 15 ? 15 : r - l - 1), l + 1);
    /* ')' 뒤: (3)state (4)ppid … (14)utime (15)stime … (20)num_threads … (24)rss */
    if (sscanf(r + 2, "%c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %ld %*d %*u %*u %ld",
               &s->state, &ut, &st, &s->threads, &rss_pages) != 5)
        return -1;
    s->cpu_ticks = ut + st;
    s->rss_kb = rss_pages * 4;
    if (with_fds) {                                     /* fd 수: Directory 를 읽어야 하므로 상대적으로 비싸다 */
        struct dirent *de;
        DIR           *d;

        snprintf(path, sizeof(path), "/proc/%d/fd", pid);
        d = opendir(path);
        s->fds = -1;
        if (d != NULL) {
            s->fds = 0;
            while ((de = readdir(d)) != NULL)
                if (de->d_name[0] != '.')
                    s->fds++;
            closedir(d);
        }
    }
    return 0;
}

#endif
