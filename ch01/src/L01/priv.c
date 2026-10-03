/* priv.c - User Mode(Ring 3)에서 특권 명령을 실행하면 어떻게 되는가 */
#include <stdio.h>

int main(void)
{
    printf("Ring 3 에서 hlt 명령을 실행합니다...\n");
    fflush(stdout);

    __asm__("hlt");     /* CPU를 멈추는 특권 명령: Ring 0 에서만 허용 */

    printf("이 줄은 출력되지 않습니다.\n");
    return 0;
}
