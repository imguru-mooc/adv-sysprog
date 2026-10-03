/* main.c - libgreet.so 의 함수를 두 번 호출한다 (Lazy Binding 관찰용) */
#include <stdio.h>
#include "greet.h"

int main(void)
{
    greet("first call");      /* 첫 호출: Dynamic Loader 가 주소를 찾는다 */
    greet("second call");     /* 둘째 호출: GOT 에 기록된 주소로 바로 간다 */
    printf("count = %d\n", greet_count());
    return 0;
}
