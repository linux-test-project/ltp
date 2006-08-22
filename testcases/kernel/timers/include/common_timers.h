/*
 * File: common_timers.h
 *
 * Keep all the common defines/checks for the timer tests here
 */

#ifndef __COMMON_TIMERS_H__
#define __COMMON_TIMERS_H__

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC (1000000000L)
#endif
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif
#ifndef CLOCK_PROCESS_CPUTIME_ID
#define CLOCK_PROCESS_CPUTIME_ID 2
#endif
#ifndef CLOCK_THREAD_CPUTIME_ID
#define CLOCK_THREAD_CPUTIME_ID 3
#endif
#ifndef CLOCK_REALTIME_HR
#define CLOCK_REALTIME_HR 4
#endif
#ifndef CLOCK_MONOTONIC_HR
#define CLOCK_MONOTONIC_HR 5
#endif
#ifndef MAX_CLOCKS
#define MAX_CLOCKS 6
#endif

#ifndef __NR_timer_create
# if defined(__i386__)
#  define __NR_timer_create 259
# elif defined(__ppc__)
#  define __NR_timer_create 240
# elif defined(__powerpc64__)
#  define __NR_timer_create 240
# elif defined(__x86_64__)
#  define __NR_timer_create 222
# else
#  error __NR_timer_create is not defined
# endif
#endif

#ifndef __NR_timer_settime
# if defined(__i386__)
#  define __NR_timer_settime (__NR_timer_create + 1)
# elif defined(__ppc__)
#  define __NR_timer_settime 241
# elif defined(__powerpc64__)
#  define __NR_timer_settime 241
# elif defined(__x86_64__)
#  define __NR_timer_settime 223
# else
#  error __NR_timer_settime is not defined
# endif
#endif

#ifndef __NR_timer_delete
# if defined(__i386__)
#  define __NR_timer_delete (__NR_timer_create + 4)
# elif defined(__ppc__)
#  define __NR_timer_delete 244
# elif defined(__powerpc64__)
#  define __NR_timer_delete 244
# elif defined(__x86_64__)
#  define __NR_timer_delete 226
# else
#  error __NR_timer_delete is not defined
# endif
#endif

#ifndef __NR_clock_settime
# if defined(__i386__)
#  define __NR_clock_settime (__NR_timer_create + 5)
# elif defined(__ppc__)
#  define __NR_clock_settime 245
# elif defined(__powerpc64__)
#  define __NR_clock_settime 245
# elif defined(__x86_64__)
#  define __NR_clock_settime 227
# else
#  error __NR_clock_settime is not defined
# endif
#endif

#ifndef __NR_clock_gettime
# if defined(__i386__)
#  define __NR_clock_gettime (__NR_timer_create + 6)
# elif defined(__ppc__)
#  define __NR_clock_gettime 246
# elif defined(__powerpc64__)
#  define __NR_clock_gettime 246
# elif defined(__x86_64__)
#  define __NR_clock_gettime 228
# else
#  error __NR_clock_gettime is not defined
# endif
#endif

/* Weak symbols. In newer glibc, these funcs should be defined. Then
 * it will superseed the definition from this file
 */
#pragma weak timer_create
#pragma weak timer_settime
#pragma weak timer_delete
#pragma weak clock_settime
#pragma weak clock_gettime

#include <time.h>
#include <sys/syscall.h>
#include <unistd.h>

int timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid)
{
	return syscall(__NR_timer_create, clockid, evp, timerid);
}
int timer_settime(timer_t timerid, int flags, const struct itimerspec *value, struct itimerspec *ovalue)
{
	return syscall(__NR_timer_settime, timerid, flags, value, ovalue);
}
int timer_delete(timer_t timerid)
{
	return syscall(__NR_timer_delete, timerid);
}
int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
	return syscall(__NR_clock_settime, clock_id, tp);
}
int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	return syscall(__NR_clock_gettime, clock_id, tp);
}

#endif
