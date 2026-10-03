/* faultlib.h - 이 Process 가 지금까지 겪은 Page Fault 횟수를 읽는다 */
#ifndef FAULTLIB_H
#define FAULTLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

static inline void faults(long *minflt, long *majflt)
{
    struct rusage ru;

    if (getrusage(RUSAGE_SELF, &ru) < 0) {
        perror("getrusage");
        exit(EXIT_FAILURE);
    }
    *minflt = ru.ru_minflt;
    *majflt = ru.ru_majflt;
}

#endif
