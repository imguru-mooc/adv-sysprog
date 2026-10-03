/* monitor.c - Linux Event & Process Monitor (Final Project)
 *
 *   하나의 epoll Loop 가 네 종류의 Event 를 처리한다
 *     timerfd  : 주기마다 대상 Process 를 Sample (/proc) → 화면 + mmap Ring Log
 *     eventfd  : Worker Thread 의 "전체 Process Scan 완료" 통지 (오래 걸리는 일은 Loop 밖에서)
 *     inotify  : monitor.conf 가 바뀌면 다시 읽는다 (Hot Reload)
 *     signalfd : SIGINT/SIGTERM → Graceful Shutdown,  SIGUSR1 → 통계 출력,  SIGUSR2 → 침입 흉내(Shell 실행 시도)
 *
 * 사용법: ./monitor [-H] [pid ...]        -H: 초기화 후 Capability 를 버리고 seccomp Filter 를 설치한다
 *         pid 를 주지 않으면 자기 자신을 감시한다. monitor.conf 의 pid= 줄도 대상에 추가된다
 */
#include "harden.h"                     /* _GNU_SOURCE 를 정의하므로 가장 먼저 */
#include "mlog.h"
#include "procstat.h"
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_TARGETS 16
#define CONF        "monitor.conf"
#define TOPN        3

enum { SRC_TIMER, SRC_SIGNAL, SRC_WORKER, SRC_INOTIFY };

struct target {
    int           pid;
    unsigned long prev_ticks;
    uint64_t      prev_ms;              /* 직전 Sample 시각: 주기가 바뀌거나 밀려도 CPU% 가 정확하다 */
    int           valid;
};

struct top_entry {
    int    pid;
    char   comm[16];
    double cpu;
};

/* ---- 상태: Event Loop Thread 만 접근하므로 Lock 이 없다 ---- */
static struct target targets[MAX_TARGETS];
static int           ntargets, cli_targets;
static int           interval_ms = 1000;
static struct mlog   mlog;
static long          hz;
static uint64_t      n_ticks, n_missed, n_scans, n_reloads;
static int           tfd;

/* ---- Worker 와 공유하는 부분: mutex 로 보호한다 ---- */
static pthread_mutex_t  top_lock = PTHREAD_MUTEX_INITIALIZER;
static struct top_entry top[TOPN];
static int              top_total;
static int              req_efd, done_efd;      /* main → worker 요청, worker → main 완료 */
static atomic_bool      stopping;

static uint64_t now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void log_event(const char *what)
{
    struct mrec r;

    memset(&r, 0, sizeof(r));
    r.ts_ms = now_ms();
    r.kind  = 'E';
    snprintf(r.comm, sizeof(r.comm), "%s", what);
    mlog_put(&mlog, &r);
}

/* ---------- Worker Thread: 모든 Process 를 두 번 Scan 해 CPU 상위 N 개를 구한다 (1 초 이상 걸린다) ---------- */
struct scan { int pid; unsigned long ticks; char comm[16]; };

static int scan_all(struct scan *out, int cap)
{
    struct dirent *de;
    DIR           *d = opendir("/proc");
    int            n = 0;

    while (d != NULL && (de = readdir(d)) != NULL && n < cap) {
        struct psample s;
        int            pid = atoi(de->d_name);

        if (pid <= 0 || ps_read(pid, &s, 0) < 0)
            continue;
        out[n].pid = pid;
        out[n].ticks = s.cpu_ticks;
        memcpy(out[n].comm, s.comm, sizeof(s.comm));
        n++;
    }
    if (d != NULL)
        closedir(d);
    return n;
}

static void *worker(void *arg)
{
    static struct scan a[4096], b[4096];
    uint64_t           v, one = 1;

    (void)arg;
    pthread_setname_np(pthread_self(), "scanner");
    while (read(req_efd, &v, sizeof(v)) == sizeof(v) && !atomic_load(&stopping)) {     /* 요청이 올 때까지 Block */
        struct top_entry best[TOPN];
        struct timespec  gap = { 1, 0 };
        int              na, nb, i, j, k;

        memset(best, 0, sizeof(best));
        na = scan_all(a, 4096);
        nanosleep(&gap, NULL);                          /* 1 초 간격의 두 Sample */
        nb = scan_all(b, 4096);
        for (i = 0; i < nb; i++)
            for (j = 0; j < na; j++) {
                double cpu;

                if (a[j].pid != b[i].pid)
                    continue;
                cpu = 100.0 * (double)(b[i].ticks - a[j].ticks) / (double)hz / 1.0;
                for (k = 0; k < TOPN; k++)
                    if (cpu > best[k].cpu) {
                        memmove(&best[k + 1], &best[k], sizeof(best[0]) * (size_t)(TOPN - 1 - k));
                        best[k].pid = b[i].pid;
                        best[k].cpu = cpu;
                        memcpy(best[k].comm, b[i].comm, sizeof(best[k].comm));
                        break;
                    }
                break;
            }
        pthread_mutex_lock(&top_lock);                  /* 결과를 넘길 때만 짧게 잠근다 */
        memcpy(top, best, sizeof(top));
        top_total = nb;
        pthread_mutex_unlock(&top_lock);
        if (write(done_efd, &one, sizeof(one)) < 0)     /* Event Loop 를 깨운다 */
            break;
    }
    return NULL;
}

/* ---------- 설정 ---------- */
static void add_target(int pid)
{
    int i;

    for (i = 0; i < ntargets; i++)
        if (targets[i].pid == pid)
            return;
    if (ntargets < MAX_TARGETS && pid > 0) {
        targets[ntargets].pid = pid;
        targets[ntargets].valid = 0;
        ntargets++;
    }
}

static void load_config(void)
{
    struct itimerspec its;
    char              line[128];
    FILE             *fp = fopen(CONF, "r");
    int               v;

    ntargets = cli_targets;                             /* 명령행에서 준 대상은 유지하고 그 뒤를 다시 채운다 */
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (sscanf(line, "interval_ms=%d", &v) == 1 && v >= 100 && v <= 60000)
                interval_ms = v;
            if (sscanf(line, "pid=%d", &v) == 1)
                add_target(v);
        }
        fclose(fp);
    }
    its.it_interval.tv_sec  = interval_ms / 1000;
    its.it_interval.tv_nsec = (interval_ms % 1000) * 1000000L;
    its.it_value = its.it_interval;
    timerfd_settime(tfd, 0, &its, NULL);
    printf("[config ] interval = %d ms, 대상 %d 개\n", interval_ms, ntargets);
}

/* ---------- Event Handler ---------- */
static void on_tick(uint64_t expirations)
{
    uint64_t one = 1;
    int      i;

    n_ticks += expirations;
    if (expirations > 1)
        n_missed += expirations - 1;                    /* Loop 가 밀렸다는 증거 (L25, L26) */

    for (i = 0; i < ntargets; i++) {
        struct psample s;
        struct mrec    r;
        double         cpu = 0;

        if (ps_read(targets[i].pid, &s, 1) < 0) {
            if (targets[i].valid)
                printf("[sample ] pid %d 가 종료되었습니다\n", targets[i].pid);
            targets[i].valid = 0;
            continue;
        }
        if (targets[i].valid && now_ms() > targets[i].prev_ms)
            cpu = 100.0 * (double)(s.cpu_ticks - targets[i].prev_ticks) / (double)hz /
                  ((double)(now_ms() - targets[i].prev_ms) / 1000.0);
        targets[i].prev_ticks = s.cpu_ticks;
        targets[i].prev_ms = now_ms();
        targets[i].valid = 1;

        printf("[sample ] %-15s pid %-6d %c cpu %5.1f%% rss %7ld kB thr %2ld fds %3d\n", s.comm, s.pid, s.state, cpu,
               s.rss_kb, s.threads, s.fds);
        memset(&r, 0, sizeof(r));
        r.ts_ms = now_ms(); r.pid = s.pid; r.cpu_x10 = (uint16_t)(cpu * 10); r.threads = (uint16_t)s.threads;
        r.rss_kb = (int32_t)s.rss_kb; r.fds = (int16_t)s.fds; r.state = s.state; r.kind = 'S';
        memcpy(r.comm, s.comm, sizeof(r.comm));
        mlog_put(&mlog, &r);                            /* Memory 에 쓰는 것으로 끝. System Call 없음 */
    }
    if (n_ticks % 3 == 0 && write(req_efd, &one, sizeof(one)) < 0)     /* 3 Tick 마다 Worker 에게 Scan 을 부탁한다 */
        perror("eventfd");
}

static void on_worker_done(void)
{
    struct top_entry t[TOPN];
    int              total, k;

    pthread_mutex_lock(&top_lock);
    memcpy(t, top, sizeof(t));
    total = top_total;
    pthread_mutex_unlock(&top_lock);
    n_scans++;
    printf("[scan   ] Process %d 개 중 CPU 상위:", total);
    for (k = 0; k < TOPN && t[k].cpu > 0.0; k++)
        printf("  %s(%d) %.0f%%", t[k].comm, t[k].pid, t[k].cpu);
    printf("\n");
}

static void print_stats(void)
{
    printf("[stats  ] tick %llu (놓친 Tick %llu), scan %llu, reload %llu, log %llu 건\n", (unsigned long long)n_ticks,
           (unsigned long long)n_missed, (unsigned long long)n_scans, (unsigned long long)n_reloads,
           (unsigned long long)mlog.hdr->total);
}

int main(int argc, char *argv[])
{
    struct signalfd_siginfo si;
    struct epoll_event      ev, events[8];
    char                    ibuf[4096];
    pthread_t               tid;
    sigset_t                mask;
    uint64_t                v, one = 1;
    int                     epfd, sfd, ifd, hardened = 0, running = 1, i, n, fds[4], srcs[4] = { SRC_TIMER, SRC_SIGNAL, SRC_WORKER, SRC_INOTIFY };

    setvbuf(stdout, NULL, _IOLBF, 0);
    hz = sysconf(_SC_CLK_TCK);
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-H") == 0)
            hardened = 1;
        else
            add_target(atoi(argv[i]));
    }
    if (ntargets == 0)
        add_target((int)getpid());
    cli_targets = ntargets;

    /* 1. Signal 은 Thread 를 만들기 전에 Block 한다 (L25) */
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT); sigaddset(&mask, SIGTERM); sigaddset(&mask, SIGUSR1); sigaddset(&mask, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    /* 2. 모든 fd 와 자원을 "권한이 있는 동안" 만든다 */
    epfd     = epoll_create1(EPOLL_CLOEXEC);
    tfd      = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    sfd      = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    done_efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    req_efd  = eventfd(0, EFD_CLOEXEC);                 /* Worker 가 Blocking read 로 기다린다 */
    ifd      = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (epfd < 0 || tfd < 0 || sfd < 0 || done_efd < 0 || req_efd < 0 || ifd < 0 ||
        inotify_add_watch(ifd, ".", IN_CLOSE_WRITE | IN_MOVED_TO) < 0 ||   /* File 이 아니라 Directory 를 감시 (L25) */
        mlog_open(&mlog, "monitor.log.bin") < 0) {
        perror("init");
        return EXIT_FAILURE;
    }
    fds[0] = tfd; fds[1] = sfd; fds[2] = done_efd; fds[3] = ifd;
    for (i = 0; i < 4; i++) {
        ev.events = EPOLLIN;
        ev.data.u32 = (uint32_t)srcs[i];
        epoll_ctl(epfd, EPOLL_CTL_ADD, fds[i], &ev);
    }
    pthread_create(&tid, NULL, worker, NULL);
    load_config();

    /* 3. 초기화가 끝났다. 이제 필요 없는 권한을 버린다 (L36, L37) */
    if (hardened) {
        if (drop_privileges() < 0) {
            perror("drop_privileges");
            return EXIT_FAILURE;
        }
        printf("[harden ] uid=%d 로 전환, Capability 는 CAP_SYS_PTRACE 와 CAP_DAC_READ_SEARCH 만 유지\n", (int)getuid());
        if (install_seccomp() < 0) {
            fprintf(stderr, "seccomp 설치 실패\n");
            return EXIT_FAILURE;
        }
        printf("[harden ] seccomp Filter 설치 완료: 이후 허용 목록 밖의 System Call 은 EPERM\n");
    }
    printf("[start  ] pid %d. 종료: Ctrl+C,  통계: kill -USR1 %d,  침입 흉내: kill -USR2 %d\n", (int)getpid(),
           (int)getpid(), (int)getpid());
    log_event("start");

    /* 4. Event Loop: 프로그램 전체에서 기다리는 곳은 여기 하나 */
    while (running) {
        n = epoll_wait(epfd, events, 8, -1);
        if (n < 0 && errno == EINTR)
            continue;
        for (i = 0; i < n; i++) {
            switch (events[i].data.u32) {
            case SRC_TIMER:
                if (read(tfd, &v, sizeof(v)) == sizeof(v))
                    on_tick(v);
                break;
            case SRC_WORKER:
                if (read(done_efd, &v, sizeof(v)) == sizeof(v))
                    on_worker_done();
                break;
            case SRC_INOTIFY: {
                ssize_t len;
                int     hit = 0;

                while ((len = read(ifd, ibuf, sizeof(ibuf))) > 0) {
                    char *p = ibuf;

                    while (p < ibuf + len) {
                        struct inotify_event *e = (struct inotify_event *)p;

                        if (e->len > 0 && strcmp(e->name, CONF) == 0)
                            hit = 1;
                        p += sizeof(*e) + e->len;
                    }
                }
                if (hit) {
                    n_reloads++;
                    load_config();
                    log_event("reload");
                }
                break;
            }
            case SRC_SIGNAL:
                while (read(sfd, &si, sizeof(si)) == (ssize_t)sizeof(si)) {
                    if (si.ssi_signo == SIGUSR1) {
                        print_stats();
                    } else if (si.ssi_signo == SIGUSR2) {
                        int rc = system("id");          /* 침입자가 Shell 을 실행하려 한다면? */

                        if (rc == -1 || (WIFEXITED(rc) && WEXITSTATUS(rc) == 127))
                            printf("[attack ] system(\"id\") 실패 (rc=%d)  ← 새 Process 를 만들 수 없다: seccomp 가 차단\n", rc);
                        else
                            printf("[attack ] system(\"id\") 가 실행되었다 (rc=%d)  ← 보호 장치가 없다\n", rc);
                    } else {
                        printf("[signal ] %u 수신 (보낸 pid %u) → Graceful Shutdown\n", si.ssi_signo, si.ssi_pid);
                        running = 0;
                    }
                }
                break;
            }
        }
    }

    /* 5. 정리: Worker 를 깨워 종료시키고, Log 를 Disk 까지 내려보낸다 */
    atomic_store(&stopping, 1);
    if (write(req_efd, &one, sizeof(one)) < 0)
        perror("eventfd");
    pthread_join(tid, NULL);
    log_event("stop");
    print_stats();
    mlog_close(&mlog);
    printf("[stop   ] 정상 종료\n");
    return EXIT_SUCCESS;
}
