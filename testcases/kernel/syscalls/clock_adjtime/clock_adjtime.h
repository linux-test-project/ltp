// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Linaro Limited. All rights reserved.
 * Author: Rafael David Tinoco <rafael.tinoco@linaro.org>
 */

#include "config.h"
#include "tst_test.h"
#include "tst_timer.h"
#include "tst_safe_clocks.h"
#include "lapi/syscalls.h"
#include "lapi/posix_clocks.h"
#include <time.h>
#include <pwd.h>
#include <sys/timex.h>
#include <sys/types.h>
#include <asm/posix_types.h>
#include "lapi/timex.h"

#ifndef __kernel_timex
struct __kernel_old_timeval {
	__kernel_old_time_t	tv_sec;		/* seconds */
	__kernel_suseconds_t	tv_usec;	/* microseconds */
};

struct __kernel_old_timex {
	unsigned int modes;	/* mode selector */
	__kernel_long_t offset;	/* time offset (usec) */
	__kernel_long_t freq;	/* frequency offset (scaled ppm) */
	__kernel_long_t maxerror;/* maximum error (usec) */
	__kernel_long_t esterror;/* estimated error (usec) */
	int status;		/* clock command/status */
	__kernel_long_t constant;/* pll time constant */
	__kernel_long_t precision;/* clock precision (usec) (read only) */
	__kernel_long_t tolerance;/* clock frequency tolerance (ppm)
				   * (read only)
				   */
	struct __kernel_old_timeval time;	/* (read only, except for ADJ_SETOFFSET) */
	__kernel_long_t tick;	/* (modified) usecs between clock ticks */

	__kernel_long_t ppsfreq;/* pps frequency (scaled ppm) (ro) */
	__kernel_long_t jitter; /* pps jitter (us) (ro) */
	int shift;              /* interval duration (s) (shift) (ro) */
	__kernel_long_t stabil;            /* pps stability (scaled ppm) (ro) */
	__kernel_long_t jitcnt; /* jitter limit exceeded (ro) */
	__kernel_long_t calcnt; /* calibration intervals (ro) */
	__kernel_long_t errcnt; /* calibration errors (ro) */
	__kernel_long_t stbcnt; /* stability limit exceeded (ro) */

	int tai;		/* TAI offset (ro) */

	int  :32; int  :32; int  :32; int  :32;
	int  :32; int  :32; int  :32; int  :32;
	int  :32; int  :32; int  :32;
};

struct __kernel_timex_timeval {
	__kernel_time64_t       tv_sec;
	long long		tv_usec;
};

struct __kernel_timex {
	unsigned int modes;	/* mode selector */
	int :32;            /* pad */
	long long offset;	/* time offset (usec) */
	long long freq;	/* frequency offset (scaled ppm) */
	long long maxerror;/* maximum error (usec) */
	long long esterror;/* estimated error (usec) */
	int status;		/* clock command/status */
	int :32;            /* pad */
	long long constant;/* pll time constant */
	long long precision;/* clock precision (usec) (read only) */
	long long tolerance;/* clock frequency tolerance (ppm)
				   * (read only)
				   */
	struct __kernel_timex_timeval time;	/* (read only, except for ADJ_SETOFFSET) */
	long long tick;	/* (modified) usecs between clock ticks */

	long long ppsfreq;/* pps frequency (scaled ppm) (ro) */
	long long jitter; /* pps jitter (us) (ro) */
	int shift;              /* interval duration (s) (shift) (ro) */
	int :32;            /* pad */
	long long stabil;            /* pps stability (scaled ppm) (ro) */
	long long jitcnt; /* jitter limit exceeded (ro) */
	long long calcnt; /* calibration intervals (ro) */
	long long errcnt; /* calibration errors (ro) */
	long long stbcnt; /* stability limit exceeded (ro) */

	int tai;		/* TAI offset (ro) */

	int  :32; int  :32; int  :32; int  :32;
	int  :32; int  :32; int  :32; int  :32;
	int  :32; int  :32; int  :32;
};
#endif

enum tst_timex_type {
	TST_KERN_OLD_TIMEX,
	TST_KERN_TIMEX
};

struct tst_timex {
	enum tst_timex_type type;
	union tx{
		struct __kernel_old_timex kern_old_timex;
		struct __kernel_timex kern_timex;
	} tx;
};

static inline void *tst_timex_get(struct tst_timex *t)
{
	switch (t->type) {
	case TST_KERN_OLD_TIMEX:
		return &t->tx.kern_old_timex;
	case TST_KERN_TIMEX:
		return &t->tx.kern_timex;
	default:
		tst_brk(TBROK, "Invalid type: %d", t->type);
		return NULL;
	}
}

static inline int sys_clock_adjtime(clockid_t clk_id, void *timex)
{
	return tst_syscall(__NR_clock_adjtime, clk_id, timex);
}

static inline int sys_clock_adjtime64(clockid_t clk_id, void *timex)
{
	return tst_syscall(__NR_clock_adjtime64, clk_id, timex);
}

#define TIMEX_SHOW(tx, mode, fmt)					\
	tst_res(TINFO,  "%s\n"						\
			"             mode: %u\n"			\
			"           offset: "fmt"\n"			\
			"        frequency: "fmt"\n"			\
			"         maxerror: "fmt"\n"			\
			"         esterror: "fmt"\n"			\
			"           status: %d (0x%x)\n"		\
			"    time_constant: "fmt"\n"			\
			"        precision: "fmt"\n"			\
			"        tolerance: "fmt"\n"			\
			"             tick: "fmt"\n"			\
			"         raw time: "fmt"(s) "fmt"(us)",	\
			mode,						\
			tx.modes,					\
			tx.offset,					\
			tx.freq,					\
			tx.maxerror,					\
			tx.esterror,					\
			tx.status,					\
			tx.status,					\
			tx.constant,					\
			tx.precision,					\
			tx.tolerance,					\
			tx.tick,					\
			tx.time.tv_sec,					\
			tx.time.tv_usec)

static inline void timex_show(const char *mode, struct tst_timex *timex)
{
	switch (timex->type) {
	case TST_KERN_OLD_TIMEX:
		TIMEX_SHOW(timex->tx.kern_old_timex, mode, "%ld");
		return;
	case TST_KERN_TIMEX:
		TIMEX_SHOW(timex->tx.kern_timex, mode, "%lld");
		return;
	default:
		tst_brk(TBROK, "Invalid type: %d", timex->type);
	}
}

#undef TIMEX_SHOW

#define ADJ_MODES	0

#define SELECT_FIELD(tx, field)						\
{									\
	switch (field) {						\
	case ADJ_MODES:							\
		return &tx.modes;					\
	case ADJ_OFFSET:						\
		return &tx.offset;					\
	case ADJ_FREQUENCY:						\
		return &tx.freq;					\
	case ADJ_MAXERROR:						\
		return &tx.maxerror;					\
	case ADJ_ESTERROR:						\
		return &tx.esterror;					\
	case ADJ_TIMECONST:						\
		return &tx.constant;					\
	case ADJ_TICK:							\
		return &tx.tick;					\
	case ADJ_STATUS:						\
		return &tx.status;					\
	default:							\
		tst_brk(TBROK, "Invalid type: %d", timex->type);	\
		return NULL;						\
	}								\
}

static inline void *timex_get_field(struct tst_timex *timex, unsigned int field)
{
	switch (timex->type) {
	case TST_KERN_OLD_TIMEX:
		SELECT_FIELD(timex->tx.kern_old_timex, field);
	case TST_KERN_TIMEX:
		SELECT_FIELD(timex->tx.kern_timex, field);
	default:
		tst_brk(TBROK, "Invalid type: %d", timex->type);
		return NULL;
	}
}

#undef SELECT_FIELD

#define TIMEX_GET_SET_FIELD_TYPE(type_libc, type_kern)			\
static inline type_kern							\
timex_get_field_##type_libc(struct tst_timex *timex, unsigned int field) \
{									\
	switch (timex->type) {						\
	case TST_KERN_OLD_TIMEX:						\
		return *((type_libc*)timex_get_field(timex, field));	\
	case TST_KERN_TIMEX:						\
		return *((type_kern*)timex_get_field(timex, field));	\
	default:							\
		tst_res(TFAIL, "Invalid type: %d", timex->type);	\
		return 0;						\
	}								\
}									\
									\
static inline void							\
timex_set_field_##type_libc(struct tst_timex *timex, unsigned int field, \
			    type_kern value)				\
{									\
	switch (timex->type) {						\
	case TST_KERN_OLD_TIMEX:						\
		*((type_libc*)timex_get_field(timex, field)) = value;	\
		return;							\
	case TST_KERN_TIMEX:						\
		*((type_kern*)timex_get_field(timex, field)) = value;	\
		return;							\
	default:							\
		tst_res(TFAIL, "Invalid type: %d", timex->type);	\
	}								\
}

TIMEX_GET_SET_FIELD_TYPE(uint, uint);
TIMEX_GET_SET_FIELD_TYPE(long, long long);

#undef TIMEX_GET_SET_FIELD_TYPE
