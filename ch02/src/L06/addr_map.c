/* addr_map.c - 변수와 함수의 주소를 출력하고 /proc/self/maps 와 대조한다 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

int        g_data = 1;
int        g_bss;
const int  g_ro = 2;

static void show_maps(void)
{
    char  line[512];
    FILE *fp = fopen("/proc/self/maps", "r");

    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    while (fgets(line, sizeof(line), fp) != NULL)
        fputs(line, stdout);
    fclose(fp);
}

int main(void)
{
    int   local = 3;
    char *heap  = malloc(100);
    void *map   = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (heap == NULL || map == MAP_FAILED) {
        perror("alloc");
        return EXIT_FAILURE;
    }

    printf("text   main    %p\n", (void *)(uintptr_t)main);
    printf("rodata g_ro    %p\n", (void *)&g_ro);
    printf("data   g_data  %p\n", (void *)&g_data);
    printf("bss    g_bss   %p\n", (void *)&g_bss);
    printf("heap   malloc  %p\n", (void *)heap);
    printf("mmap   region  %p\n", map);
    printf("libc   printf  %p\n", (void *)(uintptr_t)printf);
    printf("stack  local   %p\n", (void *)&local);
    printf("---- /proc/self/maps ----\n");
    show_maps();

    free(heap);
    munmap(map, 4096);
    return EXIT_SUCCESS;
}
