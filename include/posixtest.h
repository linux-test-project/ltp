/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * DO NOT CODE TESTS USING THIS FILE. This is only for testcases
 * merged into the LTP from the Open POSIX testsuite.
 */


#include "test.h"
#include "usctest.h"

/*
 * return codes
 */
#define PTS_PASS	0
#define PTS_FAIL        1
#define PTS_UNRESOLVED  2
#define PTS_UNSUPPORTED 4
#define PTS_UNTESTED    5
#undef TEST

/* timer_create */
#ifndef __NR_timer_create
#if defined(__i386__)
#define __NR_timer_create 259
#elif defined(__ppc__)
#define __NR_timer_create 240
#elif defined(__ppc64__)
#define __NR_timer_create 240
#elif defined(__x86_64__)
#define __NR_timer_create 222
#elif defined(__s390__)
#define __NR_timer_create 254
#elif defined(__ia64__)
#define __NR_timer_create 1248
#elif defined(__h8300__)
#define __NR_timer_create 259
#elif defined(__parisc__)
#define __NR_HPUX_timer_create 348
#endif
#endif
#pragma weak timer_create

/* timer_settime */
#ifndef __NR_timer_settime
#if defined(__parisc__)
#define __NR_HPUX_timer_settime 350
#else
#define __NR_timer_settime (__NR_timer_create+1)
#endif
#endif
#pragma weak timer_settime

/* timer_gettime */
#ifndef __NR_timer_gettime
#if defined(__parisc__)
#define __NR_HPUX_timer_gettime 351
#else
#define __NR_timer_gettime (__NR_timer_gettime+2)
#endif
#endif
#pragma weak timer_gettime

/* timer_getoverrun */
#ifndef __NR_timer_getoverrun
#if defined(__parisc__)
#define __NR_HPUX_timer_getoverrun 352
#else
#define __NR_timer_getoverrun (__NR_timer_create+3)
#endif
#endif
#pragma weak timer_getoverrun

/* timer_delete */
#ifndef __NR_timer_delete
#if defined(__parisc__)
#define __NR_HPUX_timer_delete 349
#else
#define __NR_timer_delete (__NR_timer_create+4)
#endif
#endif
#pragma weak timer_delete

/* clock_settime */
#ifndef __NR_clock_settime
#if defined(__parisc__)
#define __NR_HPUX_clock_settime 345
#else
#define __NR_clock_settime (__NR_timer_create+5)
#endif
#endif
#pragma weak clock_settime

/* clock_gettime */
#ifndef __NR_clock_gettime
#if defined(__parisc__)
#define __NR_HPUX_clock_gettime 346
#else
#define __NR_clock_gettime (__NR_timer_create+6)
#endif
#endif
#pragma weak clock_gettime

/* clock_getres */
#ifndef __NR_clock_getres
#if defined(__parisc__)
#define __NR_HPUX_clock_getres 347
#else
#define __NR_clock_getres (__NR_timer_create+7)
#endif
#endif
#pragma weak clock_getres

/* clock_nanosleep */
#ifndef __NR_clock_nanoslee
#define __NR_clock_nanosleep       (__NR_timer_create+8)
#endif
#pragma weak clock_nanosleep
