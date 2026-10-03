/* mlog.h - mmap 으로 만든 고정 크기 Ring Log. write() System Call 없이 Memory 에 쓰는 것으로 기록이 끝난다 (L10, L14, L15) */
#ifndef MLOG_H
#define MLOG_H

#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MLOG_MAGIC 0x4C535032u          /* "LSP2" */
#define MLOG_CAP   4096                 /* Record 수. 가득 차면 오래된 것부터 덮어쓴다 */

struct mrec {
    uint64_t ts_ms;                     /* CLOCK_REALTIME, ms */
    int32_t  pid;
    uint16_t cpu_x10;                   /* CPU% × 10 */
    uint16_t threads;
    int32_t  rss_kb;
    int16_t  fds;
    char     state;
    char     kind;                      /* 'S' Sample, 'E' Event(설정 변경, 종료 등) */
    char     comm[16];
};                                      /* 40 Byte */

struct mlog_hdr {
    uint32_t magic, cap;
    uint64_t total;                     /* 지금까지 기록한 Record 수 (계속 증가. 위치는 total % cap) */
};

struct mlog {
    struct mlog_hdr *hdr;
    struct mrec     *rec;
    size_t           bytes;
    int              fd;
};

static inline int mlog_open(struct mlog *m, const char *path)
{
    m->bytes = sizeof(struct mlog_hdr) + sizeof(struct mrec) * MLOG_CAP;
    m->fd = open(path, O_RDWR | O_CREAT, 0644);
    if (m->fd < 0 || ftruncate(m->fd, (off_t)m->bytes) < 0)     /* Mapping 전에 File 크기를 확보한다 (아니면 SIGBUS) */
        return -1;
    m->hdr = mmap(NULL, m->bytes, PROT_READ | PROT_WRITE, MAP_SHARED, m->fd, 0);
    if (m->hdr == MAP_FAILED)
        return -1;
    m->rec = (struct mrec *)(m->hdr + 1);
    if (m->hdr->magic != MLOG_MAGIC) {                          /* 새 File */
        m->hdr->magic = MLOG_MAGIC;
        m->hdr->cap   = MLOG_CAP;
        m->hdr->total = 0;
    }
    return 0;
}

static inline void mlog_put(struct mlog *m, const struct mrec *r)
{
    m->rec[m->hdr->total % MLOG_CAP] = *r;              /* 내용을 먼저 쓰고 */
    m->hdr->total++;                                    /* 그 다음에 공개한다 (L28 과 같은 순서) */
}

static inline void mlog_close(struct mlog *m)
{
    msync(m->hdr, m->bytes, MS_SYNC);                   /* 종료 시에는 Disk 기록까지 기다린다 (L15) */
    munmap(m->hdr, m->bytes);
    close(m->fd);
}

#endif
