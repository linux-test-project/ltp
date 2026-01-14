// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *               Author: William Roske
 * Copyright (c) Linux Test Project, 2026
 */

#ifndef TSO_USCTEST_H__
#define TSO_USCTEST_H__

/* For PATH_MAX */
#include <linux/limits.h>

/***********************************************************************
 * The following globals are defined in parse_opts.c but must be
 * externed here because they are used in the macros defined below.
 ***********************************************************************/
extern int STD_LOOP_COUNT; /* changed by -in to set loop count to n */

extern long TEST_RETURN;
extern int TEST_ERRNO;

/***********************************************************************
 * TEST: calls a system call
 *
 * parameters:
 *	SCALL = system call and parameters to execute
 *
 ***********************************************************************/
#define TEST(SCALL) \
	do { \
		errno = 0; \
		TEST_RETURN = SCALL; \
		TEST_ERRNO = errno; \
	} while (0)

/***********************************************************************
 * TEST_PAUSE: Pause for SIGUSR1 if the pause flag is set.
 *	       Just continue when signal comes in.
 *
 * parameters:
 *	none
 *
 ***********************************************************************/
#define TEST_PAUSE usc_global_setup_hook();
int usc_global_setup_hook();

/***********************************************************************
 * TEST_LOOPING now call the usc_test_looping function.
 * The function will return 1 if the test should continue
 * iterating.
 *
 ***********************************************************************/
#define TEST_LOOPING usc_test_looping
int usc_test_looping(int counter);

#endif /* TSO_USCTEST_H__ */
