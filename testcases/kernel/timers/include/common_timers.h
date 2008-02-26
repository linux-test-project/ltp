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
clock_t clock_list[] = {
	CLOCK_REALTIME,
	CLOCK_MONOTONIC,
	CLOCK_PROCESS_CPUTIME_ID,
	CLOCK_THREAD_CPUTIME_ID
};
#define MAX_CLOCKS (sizeof(clock_list) / sizeof(*clock_list))

const char *get_clock_str(const int clock_id)
{
	switch(clock_id) {
		case CLOCK_REALTIME:           return "CLOCK_REALTIME";
		case CLOCK_MONOTONIC:          return "CLOCK_MONOTONIC";
		case CLOCK_PROCESS_CPUTIME_ID: return "CLOCK_PROCESS_CPUTIME_ID";
		case CLOCK_THREAD_CPUTIME_ID:  return "CLOCK_THREAD_CPUTIME_ID";
		default:                       return "CLOCK_!?!?!?";
	}
}

#include "linux_syscall_numbers.h"

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
