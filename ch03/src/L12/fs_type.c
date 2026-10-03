/* fs_type.c - 경로가 어느 File System 에 속하는지 statfs 의 Magic Number 로 확인한다
 * 사용법: ./fs_type [경로 ...]
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/vfs.h>

static const char *fs_name(unsigned long magic)
{
    switch (magic) {
    case 0xEF53:     return "ext2/3/4";
    case 0x9fa0:     return "proc";
    case 0x62656572: return "sysfs";
    case 0x01021994: return "tmpfs";
    case 0x1cd1:     return "devpts";
    case 0x58465342: return "xfs";
    case 0x9123683E: return "btrfs";
    case 0x794c7630: return "overlayfs";
    case 0x6969:     return "nfs";
    case 0x74726163: return "tracefs";
    case 0x27e0eb:   return "cgroup";
    case 0x63677270: return "cgroup2";
    default:         return "?";
    }
}

int main(int argc, char *argv[])
{
    const char *defaults[] = { "/", "/home", "/tmp", "/proc", "/sys", "/dev", "/run", "/sys/fs/cgroup" };
    int         i, n = argc > 1 ? argc - 1 : (int)(sizeof(defaults) / sizeof(defaults[0]));

    for (i = 0; i < n; i++) {
        const char   *path = argc > 1 ? argv[i + 1] : defaults[i];
        struct statfs sf;

        if (statfs(path, &sf) < 0) {
            perror(path);
            continue;
        }
        printf("%-16s magic=0x%-10lx %-10s block=%ld\n", path,
               (unsigned long)sf.f_type, fs_name((unsigned long)sf.f_type), (long)sf.f_bsize);
    }
    return EXIT_SUCCESS;
}
