
  /*
   * Copyright (c) 2002, Intel Corporation. All rights reserved.
   * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
   * This file is licensed under the GPL license.  For the full content
   * of this license, see the COPYING file at the top level of this
   * source tree.
   *
   * Test the following symbols are defined by pthread.h
   *
   * This is done with functions, not #ifdefs because the spec doesn't state
   * that they have to be #define's, and instead can be values in an enum, like
   * on FreeBSD.
   */

#include <pthread.h>

#define TEST_MACRO(CONSTANT) \
	void test_ ## CONSTANT(void) { int dummy = CONSTANT; (void)dummy; }

/* BAR */
TEST_MACRO(PTHREAD_BARRIER_SERIAL_THREAD)

/* XSI */
    TEST_MACRO(PTHREAD_MUTEX_DEFAULT)

/* XSI */
    TEST_MACRO(PTHREAD_MUTEX_ERRORCHECK)

/* XSI */
    TEST_MACRO(PTHREAD_MUTEX_NORMAL)

/* XSI */
    TEST_MACRO(PTHREAD_MUTEX_RECURSIVE)

/* TPP|TPI */
    TEST_MACRO(PTHREAD_PRIO_INHERIT)

/* TPP|TPI */
    TEST_MACRO(PTHREAD_PRIO_NONE)

/* TPP|TPI */
    TEST_MACRO(PTHREAD_PRIO_PROTECT)

/* TPS */
    TEST_MACRO(PTHREAD_SCOPE_PROCESS)

/* TPS */
    TEST_MACRO(PTHREAD_SCOPE_SYSTEM)
