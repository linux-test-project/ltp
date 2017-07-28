/*
 * File: common_timers.h
 *
 * Keep all the common defines/checks for the timer tests here
 */

#ifndef __COMMON_TIMERS_H__
#define __COMMON_TIMERS_H__

#define CLEANUP cleanup
#include "config.h"
#include "lapi/syscalls.h"

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC (1000000000L)
#endif
clock_t clock_list[] = {
	CLOCK_REALTIME,
	CLOCK_MONOTONIC,
	CLOCK_PROCESS_CPUTIME_ID,
	CLOCK_THREAD_CPUTIME_ID,
#if HAVE_CLOCK_MONOTONIC_RAW
	CLOCK_MONOTONIC_RAW,
#endif
#if HAVE_CLOCK_REALTIME_COARSE
	CLOCK_REALTIME_COARSE,
#endif
#if HAVE_CLOCK_MONOTONIC_COARSE
	CLOCK_MONOTONIC_COARSE,
#endif
};
/* CLOCKS_DEFINED is the number of clock sources defined for sure */
#define CLOCKS_DEFINED (sizeof(clock_list) / sizeof(*clock_list))
/* MAX_CLOCKS is the maximum number of clock sources supported by kernel */
#define MAX_CLOCKS 16

#define CLOCK_TO_STR(def_name)	\
	case def_name:		\
		return #def_name;

const char *get_clock_str(const int clock_id)
{
	switch(clock_id) {
	CLOCK_TO_STR(CLOCK_REALTIME);
	CLOCK_TO_STR(CLOCK_MONOTONIC);
	CLOCK_TO_STR(CLOCK_PROCESS_CPUTIME_ID);
	CLOCK_TO_STR(CLOCK_THREAD_CPUTIME_ID);
#if HAVE_CLOCK_MONOTONIC_RAW
	CLOCK_TO_STR(CLOCK_MONOTONIC_RAW);
#endif
#if HAVE_CLOCK_REALTIME_COARSE
	CLOCK_TO_STR(CLOCK_REALTIME_COARSE);
#endif
#if HAVE_CLOCK_MONOTONIC_COARSE
	CLOCK_TO_STR(CLOCK_MONOTONIC_COARSE);
#endif
	default:
		return "CLOCK_!?!?!?";
	}
}

#include "lapi/syscalls.h"

#include <time.h>
#include <unistd.h>

/* timer_t in kernel(int) is different from  Glibc definition(void*).
 * Use the kernel definition for syscall tests
 */
typedef int kernel_timer_t;

#endif
