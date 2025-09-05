/*
 * File: common_timers.h
 *
 * Keep all the common defines/checks for the timer tests here
 */

#ifndef LAPI_COMMON_TIMERS_H__
#define LAPI_COMMON_TIMERS_H__

#include <linux/version.h>
#include "config.h"
#include "lapi/syscalls.h"
#include "lapi/posix_clocks.h"

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC (1000000000LL)
#endif

static const clock_t clock_list[] = {
	CLOCK_REALTIME,
	CLOCK_MONOTONIC,
	CLOCK_PROCESS_CPUTIME_ID,
	CLOCK_THREAD_CPUTIME_ID,
	CLOCK_BOOTTIME,
	CLOCK_BOOTTIME_ALARM,
	CLOCK_REALTIME_ALARM,
	CLOCK_TAI,
};
/* CLOCKS_DEFINED is the number of clock sources defined for sure */
#define CLOCKS_DEFINED (sizeof(clock_list) / sizeof(*clock_list))
/* MAX_CLOCKS is the maximum number of clock sources supported by kernel */
#define MAX_CLOCKS 16

#define MAX_AUX_CLOCKS 8

#define CLOCK_TO_STR(def_name)	\
	case def_name:		\
		return #def_name;

static inline const char *get_clock_str(const int clock_id)
{
	switch (clock_id) {
	CLOCK_TO_STR(CLOCK_REALTIME);
	CLOCK_TO_STR(CLOCK_MONOTONIC);
	CLOCK_TO_STR(CLOCK_PROCESS_CPUTIME_ID);
	CLOCK_TO_STR(CLOCK_THREAD_CPUTIME_ID);
	CLOCK_TO_STR(CLOCK_BOOTTIME);
	CLOCK_TO_STR(CLOCK_BOOTTIME_ALARM);
	CLOCK_TO_STR(CLOCK_REALTIME_ALARM);
	CLOCK_TO_STR(CLOCK_TAI);
	default:
		return "CLOCK_!?!?!?";
	}
}

static inline int possibly_unsupported(clock_t clock)
{
	switch (clock) {
	case CLOCK_BOOTTIME:
	case CLOCK_BOOTTIME_ALARM:
	case CLOCK_REALTIME_ALARM:
	case CLOCK_TAI:
			return 1;
	default:
			return 0;
	}
}

#include "lapi/syscalls.h"

#include <time.h>
#include <unistd.h>

/* timer_t in kernel(int) is different from  Glibc definition(void*).
 * Use the kernel definition for syscall tests
 */
typedef int kernel_timer_t;

#endif /* LAPI_COMMON_TIMERS_H__ */
