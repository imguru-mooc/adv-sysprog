/* sections.c - 변수와 함수가 ELF의 어느 Section에 들어가는지 확인한다 */
#include <stdint.h>
#include <stdio.h>

int         g_init   = 42;          /* .data   : 초기값이 있는 전역 변수 */
int         g_uninit;               /* .bss    : 초기값이 없는 전역 변수 */
const int   g_const  = 7;           /* .rodata : 읽기 전용 */
static int  s_init   = 100;         /* .data   : static 도 동일 */
static char s_buf[4096];            /* .bss */
const char *g_str    = "string literal";   /* 포인터는 .data, 문자열은 .rodata */

static void func(void) { }          /* .text */

int main(void)
{
    int local = 1;                  /* Stack: ELF File 에는 없다 */

    printf(".text    func     %p\n", (void *)(uintptr_t)func);
    printf(".rodata  g_const  %p\n", (void *)&g_const);
    printf(".rodata  literal  %p\n", (void *)g_str);
    printf(".data    g_init   %p\n", (void *)&g_init);
    printf(".data    s_init   %p\n", (void *)&s_init);
    printf(".bss     g_uninit %p\n", (void *)&g_uninit);
    printf(".bss     s_buf    %p\n", (void *)s_buf);
    printf("stack    local    %p\n", (void *)&local);
    return 0;
}
