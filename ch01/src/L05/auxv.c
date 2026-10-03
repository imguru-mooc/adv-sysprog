/* auxv.c - Kernel 이 execve 때 Stack 에 넣어 준 Auxiliary Vector 를 읽는다 */
#include <elf.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/auxv.h>

int main(void)
{
    printf("AT_PAGESZ  %lu\n", getauxval(AT_PAGESZ));
    printf("AT_PHDR    0x%lx   (Program Header Table 이 Mapping 된 주소)\n", getauxval(AT_PHDR));
    printf("AT_PHNUM   %lu      (Program Header 개수)\n", getauxval(AT_PHNUM));
    printf("AT_BASE    0x%lx   (Dynamic Loader 가 Mapping 된 주소)\n", getauxval(AT_BASE));
    printf("AT_ENTRY   0x%lx   (이 프로그램의 Entry Point = _start)\n", getauxval(AT_ENTRY));
    printf("AT_EXECFN  %s\n", (const char *)(uintptr_t)getauxval(AT_EXECFN));
    printf("AT_RANDOM  0x%lx   (Stack Protector 용 Random 16 Bytes 의 주소)\n", getauxval(AT_RANDOM));
    printf("AT_SYSINFO_EHDR 0x%lx   (vDSO 주소)\n", getauxval(AT_SYSINFO_EHDR));
    return 0;
}
