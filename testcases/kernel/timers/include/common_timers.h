/*
 * File: common_timers.h
 *
 * Keep all the common defines/checks for the timer tests here
 */

#ifndef __COMMON_TIMERS_H__
#define __COMMON_TIMERS_H__

#define CLEANUP cleanup
#include "config.h"
#include "linux_syscall_numbers.h"

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
#define MAX_CLOCKS (sizeof(clock_list) / sizeof(*clock_list))

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

#include "linux_syscall_numbers.h"

#include <time.h>
#include <unistd.h>

#endif
