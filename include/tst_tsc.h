// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright © International Business Machines  Corp., 2006-2008
 *
 * AUTHOR
 *        Darren Hart <dvhltc@us.ibm.com>
 *        Giuseppe Cavallaro <peppe.cavallarost.com>
 *
 * HISTORY
 *      It directly comes from the librttest.h (see its HISTORY).
 */

#ifndef TST_TSC_H
#define TST_TSC_H

#undef TSC_UNSUPPORTED

/* TSC macros */
#if defined(__i386__)
#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))
#elif defined(__x86_64__)
#define rdtscll(val)					\
	do {						\
		uint32_t low, high;			\
		__asm__ __volatile__ ("rdtsc" : "=a" (low), "=d" (high)); \
		val = (uint64_t)high << 32 | low;	\
	} while (0)
#elif defined(__powerpc__)
#if defined(__powerpc64__)	/* 64bit version */
#define rdtscll(val)					\
	do {								\
		__asm__ __volatile__ ("mfspr %0, 268" : "=r" (val));	\
	} while (0)
#else	/*__powerpc__ 32bit version */
#define rdtscll(val)							\
	 do {								\
		uint32_t tbhi, tblo ;					\
		__asm__ __volatile__ ("mftbu %0" : "=r" (tbhi));	\
		__asm__ __volatile__ ("mftbl %0" : "=r" (tblo));	\
		val = 1000 * ((uint64_t) tbhi << 32) | tblo;		\
	} while (0)
#endif
#else
#warning TSC UNSUPPORTED
/* All tests will be compiled also for the
 * architecture without TSC support (e.g. SH).
 * At run-time these will fail with ENOTSUP.
 */
#define rdtscll(val)	do {  } while (0)
#define TSC_UNSUPPORTED
#endif

#endif
