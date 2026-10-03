/* va_split.c - 48bit Virtual Address 를 4-Level Page Table Index 와 Offset 으로 분해한다 */
#include <stdio.h>
#include <stdlib.h>

static void split(unsigned long va)
{
    printf("VA            = 0x%016lx\n", va);
    printf("  PGD index   = %3lu   (bit 47..39)\n", (va >> 39) & 0x1ff);
    printf("  PUD index   = %3lu   (bit 38..30)\n", (va >> 30) & 0x1ff);
    printf("  PMD index   = %3lu   (bit 29..21)\n", (va >> 21) & 0x1ff);
    printf("  PTE index   = %3lu   (bit 20..12)\n", (va >> 12) & 0x1ff);
    printf("  Offset      = 0x%03lx (bit 11..0)\n", va & 0xfff);
    printf("  VPN         = 0x%lx\n", va >> 12);
}

int main(int argc, char *argv[])
{
    int local = 0;

    if (argc > 1) {
        split(strtoul(argv[1], NULL, 16));
        return EXIT_SUCCESS;
    }
    printf("[stack 의 지역 변수]\n");
    split((unsigned long)&local);
    printf("\n사용법: %s <16진수 주소>\n", argv[0]);
    return EXIT_SUCCESS;
}
