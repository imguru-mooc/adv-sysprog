/* procmon.c - /proc 만 읽어서 만드는 작은 top. Final Project 의 Process Monitoring 부분
 * 사용법: ./procmon <pid> [횟수=5]
 *   CPU% : /proc/PID/stat 의 utime+stime 을 1 초 간격으로 두 번 읽어 차이를 구한다
 */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct sample {
    unsigned long utime, stime;         /* 단위: clock tick (보통 1/100 초) */
    long          threads, rss_kb, vm_kb, vol_cs, nonvol_cs;
    unsigned long rchar, wchar, syscr, syscw, write_bytes;
    int           fds;
    char          state;
};

static int read_stat(int pid, struct sample *s)
{
    char  path[64], buf[1024], *p;
    FILE *fp;

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fp = fopen(path, "r");
    if (fp == NULL)
        return -1;
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    p = strrchr(buf, ')');                              /* comm 에 공백이나 ')' 가 있을 수 있으므로 마지막 ')' 뒤부터 */
    if (p == NULL)
        return -1;
    /* (3)state ... (14)utime (15)stime : ')' 뒤의 첫 필드가 3 번 */
    if (sscanf(p + 2, "%c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &s->state, &s->utime, &s->stime) != 3)
        return -1;
    return 0;
}

static void read_status(int pid, struct sample *s)
{
    char  path[64], line[256];
    FILE *fp;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    fp = fopen(path, "r");
    if (fp == NULL)
        return;
    while (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "Threads: %ld", &s->threads);
        sscanf(line, "VmRSS: %ld kB", &s->rss_kb);
        sscanf(line, "VmSize: %ld kB", &s->vm_kb);
        sscanf(line, "voluntary_ctxt_switches: %ld", &s->vol_cs);
        sscanf(line, "nonvoluntary_ctxt_switches: %ld", &s->nonvol_cs);
    }
    fclose(fp);
}

static void read_io(int pid, struct sample *s)
{
    char  path[64], line[128];
    FILE *fp;

    snprintf(path, sizeof(path), "/proc/%d/io", pid);   /* 자기 Process 가 아니면 권한이 필요할 수 있다 */
    fp = fopen(path, "r");
    if (fp == NULL)
        return;
    while (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "rchar: %lu", &s->rchar);
        sscanf(line, "wchar: %lu", &s->wchar);
        sscanf(line, "syscr: %lu", &s->syscr);
        sscanf(line, "syscw: %lu", &s->syscw);
        sscanf(line, "write_bytes: %lu", &s->write_bytes);
    }
    fclose(fp);
}

static int count_fds(int pid)
{
    struct dirent *de;
    char           path[64];
    DIR           *d;
    int            n = 0;

    snprintf(path, sizeof(path), "/proc/%d/fd", pid);
    d = opendir(path);
    if (d == NULL)
        return -1;
    while ((de = readdir(d)) != NULL)
        if (de->d_name[0] != '.')
            n++;
    closedir(d);
    return n;
}

static int take(int pid, struct sample *s)
{
    memset(s, 0, sizeof(*s));
    if (read_stat(pid, s) < 0)
        return -1;
    read_status(pid, s);
    read_io(pid, s);
    s->fds = count_fds(pid);
    return 0;
}

int main(int argc, char *argv[])
{
    struct sample a, b;
    long          hz = sysconf(_SC_CLK_TCK);
    int           pid, n, i;

    if (argc < 2) {
        fprintf(stderr, "사용법: %s <pid> [횟수]\n", argv[0]);
        return EXIT_FAILURE;
    }
    pid = atoi(argv[1]);
    n   = argc > 2 ? atoi(argv[2]) : 5;
    if (take(pid, &a) < 0) {
        perror("/proc/PID/stat");
        return EXIT_FAILURE;
    }
    printf("CLK_TCK = %ld\n", hz);
    printf("%4s %2s %6s %4s %8s %8s %4s %8s %8s %8s\n", "sec", "S", "CPU%", "thr", "RSS(kB)", "VSZ(kB)", "fds", "write/s", "volCS/s", "invCS/s");
    for (i = 1; i <= n; i++) {
        sleep(1);
        if (take(pid, &b) < 0) {
            printf("Process 가 종료되었습니다\n");
            break;
        }
        printf("%4d %2c %6.1f %4ld %8ld %8ld %4d %8lu %8ld %8ld\n", i, b.state,
               100.0 * (double)((b.utime + b.stime) - (a.utime + a.stime)) / (double)hz,
               b.threads, b.rss_kb, b.vm_kb, b.fds, b.syscw - a.syscw, b.vol_cs - a.vol_cs, b.nonvol_cs - a.nonvol_cs);
        a = b;
    }
    return EXIT_SUCCESS;
}
