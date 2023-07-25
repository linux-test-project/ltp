/*
 *
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

/******************************************************************************

   File:        epoll-ltp.c

   Description:
     Test the epoll_* system calls. This test program attempts to
     be very thorough in exercising epoll_* system calls. Large
     combinations of valid and invalid parameters are passed with
     valid and invalid sequences. Due to the combinatorial nature
     of this test program the test may take a "long" time to
     execute.

   Total Tests: 2 (2 system calls are being tested for)

   Test Name:   epoll_create, epoll_ctl

   Test Assertion
   & Strategy:  Test a variety of incorrect parameters for epoll_create

                Then run a reasonable epoll_create and get a fd for the epoll
                     set.

                Next run epoll_ctl on that fd (epoll_fd) with a variety of
                     incorrect parameters and a couple correct ones.

                Finally ?How to thoroughly test epoll_wait?

   Author:      Matt Helsley <matthltc@us.ibm.com>

   History:     Created - May 22 2003 - Matt Helsley <matthltc@us.ibm.com>
                Added   -

   Notes: Currently we assume that the OS will never allocate an fd s.t.
          fd == INT_MAX and that it will instead choose to allocate fds
          from the "low" numbers. -MH

   Currently pokes epoll_create several times in 2 + NUM_RAND_ATTEMPTS ways
             pokes epoll_ctl 27648 - (2 + NUM_RAND_ATTEMPTS) ways
             does not poke epoll_wait

   TODO: change errno test code to build lists of possible errno values for
            each erroneous parameter. Check that the errno value is in one
            of the lists. Currently errno is not checked at all when multiple
            erroneous parameters are passed in.

         test epoll_ctl with a large number of file descriptor events in the
            set

   Link against epoll and ltp (-lepoll -lltp)

*******************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "config.h"
#include "test.h"

char *TCID = "epoll01";
int TST_TOTAL = 1;

#ifdef HAVE_SYS_EPOLL_H

#include <sys/epoll.h>

/* Local Defines */
#if !defined(TRUE) && !defined(FALSE)
#define TRUE  1
#define FALSE 0
#endif

#define NUM_RAND_ATTEMPTS 16
#define BACKING_STORE_SIZE_HINT 32

/*
  Define the beginning of a "protected region".
  This is a region where a wide variety of errors
  could occur or signals could arrive (including
  SIGSEGV and SIGKILL).
$
  The test program uses this to catch those
  conditions as best it can and continue testing.

  The region MUST be marked by a corresponding
  PROTECT_REGION_END.

  DO NOT nest protected regions! i.e. Do not build
  code of the form:

	PROTECT_REGION_START
              ...
	PROTECT_REGION_START
              ...
	PROTECT_REGION_END
              ...
	PROTECT_REGION_END
 */
#define PROTECT_REGION_START		\
do {					\
	pid_t kid_pid;			\
	int kid_status;			\
					\
	tst_old_flush();			\
	kid_pid = tst_fork();	\
	if (kid_pid == 0) {

#define PROTECT_REGION_EXIT(errval) return (errval);

#define PROTECT_REGION_END(result, errval)					\
	return 0;								\
	} else {								\
	 waitpid(kid_pid, &kid_status, 0);					\
	 if (WIFEXITED(kid_status)) {						\
		(result) = WEXITSTATUS(kid_status);				\
	} else { /* Must have been signaled */					\
		(result) = (errval);						\
		if (WIFSIGNALED(kid_status))						\
			tst_resm(TFAIL, "Protected function test exited due to signal %d (%s)", \
				WTERMSIG(kid_status), strsignal(WTERMSIG(kid_status)));	\
		}								\
	}									\
} while (0)

/*
 * Call a function in a "protected" context.
 * This protects the test program proper from segfaults
 * and allows for the extraction of an integer return
 * code.
 *
 * return only integer results.
 */
#define PROTECT_FUNC(fn, errval, epoll_fd) (					\
{										\
	pid_t kid_pid;								\
	int kid_status;								\
										\
	tst_old_flush();								\
	kid_pid = tst_fork();						\
	if (kid_pid == 0) { /* Run the function */				\
		return fn(epoll_fd);						\
	} else {								\
		waitpid(kid_pid, &kid_status, 0);				\
		if (WIFEXITED(kid_status)) {					\
		kid_status = WEXITSTATUS(kid_status);				\
	} else { /* Must have been signaled */					\
		kid_status = (errval);						\
		if (WIFSIGNALED(kid_status))					\
			tst_resm(TFAIL, "Protected function test exited due to signal %d (%s)", \
						WTERMSIG(kid_status), strsignal(WTERMSIG(kid_status))); \
	}									\
}										\
kid_status = kid_status;})

/*
 * Given the number of random size requests to test,
 * test various boundary cases of epoll_create().
 *
 * Return the number of tests that failed. 0 indicates
 * 100% passed.
 */
int test_epoll_create(unsigned int num_rand_attempts)
{
	int epoll_fd = -1;
	int fd_set_size = -1;
	unsigned int attempt_count;
	unsigned int num_epoll_create_test_fails = 0;
	unsigned int num_epoll_create_test_calls = 0;

	/* Negative set sizes */
	errno = 0;
	fd_set_size = -1;
	num_epoll_create_test_calls++;
	epoll_fd = epoll_create(fd_set_size);
	if (epoll_fd >= 0) {
		tst_resm(TFAIL | TERRNO,
			 "epoll_create with negative set size succeeded unexpectedly");
		num_epoll_create_test_fails++;
		close(epoll_fd);
	} else {
		if (errno != EINVAL) {
			tst_resm(TFAIL | TERRNO,
				 "epoll_create with negative set size didn't set errno to EINVAL");
			num_epoll_create_test_fails++;
		} else {
			tst_resm(TPASS, "epoll_create with negative set size");
		}
	}

	/* Large set sizes -- try several less than or equal to INT_MAX by some
	   small amount (expect num_rand_attempts to be approximately the
	   amount we'd like to go below INT_MAX). */
	fd_set_size = INT_MAX;
	for (attempt_count = num_rand_attempts; attempt_count > 0;
	     attempt_count--, fd_set_size--) {
		num_epoll_create_test_calls++;
		epoll_fd = epoll_create(fd_set_size);
		if (epoll_fd == -1) {
			if (errno != ENOMEM) {
				tst_resm(TFAIL,
					 "epoll_create with large set size (size = %d)",
					 fd_set_size);
				num_epoll_create_test_fails++;
			} else {
				tst_resm(TPASS,
					 "epoll_create with large set size (size = %d)",
					 fd_set_size);
			}
		} else {
			tst_resm(TPASS,
				 "epoll_create with large set size (size = %d)",
				 fd_set_size);
			close(epoll_fd);
		}
	}

	/* Random large set sizes */
	for (attempt_count = num_rand_attempts; attempt_count > 0;
	     attempt_count--) {
		fd_set_size = abs(rand() + SHRT_MAX) % INT_MAX;
		errno = 0;
		num_epoll_create_test_calls++;
		epoll_fd = epoll_create(fd_set_size);
		if (epoll_fd < 0) {
			if (errno != ENOMEM) {
				tst_resm(TFAIL,
					 "epoll_create with random random large set size (size = %d)",
					 fd_set_size);
				num_epoll_create_test_fails++;
			} else {
				tst_resm(TPASS,
					 "epoll_create with random random large set size (size = %d)",
					 fd_set_size);
			}
		} else {
			tst_resm(TPASS,
				 "epoll_create with random large set size (size = %d)",
				 fd_set_size);
			close(epoll_fd);
		}
	}

	tst_resm(TINFO,
		 "Summary: Of %d tests, epoll_create failed %d (%3.0f%% passed).",
		 num_epoll_create_test_calls, num_epoll_create_test_fails,
		 ((float)
		  (num_epoll_create_test_calls - num_epoll_create_test_fails)
		  * 100.0f / (float)
		  num_epoll_create_test_calls));
	/* Return 0 on success. */

	return num_epoll_create_test_fails;
}

/* RES_PASS indicates a PASS result */
#define RES_PASS 0

/*
 * RES_FAIL_* indicates a FAIL result
 * In brief, there are two things that can go wrong in a
 * failure. The return value (result = epoll_ctl(...)) and
 * the errno value may not match expectations. In this notation,
 * MIS -> mismatch, MAT -> match, BAD -> bad, and IGN -> ignored.
 *
 * RETV_MIS_* indicates the return value was either 0 or 1, but did
 * not match the expected return value
 *
 * _RETV_MAT_* indicates that the return value was 0 xor 1 and did
 * match the expected value
 *
 *_RETV_BAD_* the return value was neither 0 nor 1.
 *_ERRNO_MAT  the error number matched the expected number
 *_ERRNO_MIS  the error number did not match the expected number
 *_ERRNO_IGN  no error number was expected and so errno was ignored
 *
 * Keep these values below 256 as only 1 byte gets passed as a
 * return value for the process. Note that RES_PASS is 0 which
 * LTP interprets as a PASS.
 */

/* Did not get the expected return value, but errno value was expected */
#define RES_FAIL_RETV_MIS_ERRNO_MAT 1
/* Did not get the expected return value, but errno value was expected */
#define RES_FAIL_RETV_BAD_ERRNO_MAT 2
/* Did get the expected return value, and errno value was not expected */
#define RES_FAIL_RETV_MAT_ERRNO_MIS 3
/* Return value was neither 0 nor -1. Mismatch in value of errno */
#define RES_FAIL_RETV_BAD_ERRNO_MIS 4
/* Did not get the expected return value and errno is irrelevant */
#define RES_FAIL_RETV_MIS_ERRNO_IGN 5
/* Return value was neither 0 nor -1. value of errno is irrelevant */
#define RES_FAIL_RETV_BAD_ERRNO_IGN 6
/* We expected multiple errors so we were unable to check errno for conformance */
#define RES_PASS_RETV_MAT_ERRNO_IGN 7

static const char *result_strings[] = {
	"Passed",
	"Return value mismatched yet errno matched.",
	"Return value was bad    yet errno matched.",
	"Return value matched    yet errno mismatched.",
	"Return value was bad    and errno mismatched.",
	"Return value mismatched  so errno ignored.",
	"Return value was bad     so errno ignored.",
	"Return value matched    but errno ignored. (multiple errors expected)"
};

/****************************************************************************************/
/* This macro helps keep the code below understandable. It prints out the
   failure message passed to it plus the parameters to the system call. */
#define EPOLL_CTL_TEST_RESULTS_SHOW_PARAMS 1
#if EPOLL_CTL_TEST_RESULTS_SHOW_PARAMS
#define EPOLL_CTL_TEST_FAIL(msg , ...) \
({ \
	if (ev_ptr != NULL) { \
		tst_resm(TFAIL, ( "(epoll_ctl(%d,%08x,%d,%p = {%08x,%08d}) returned %d:%s)" ) , ##__VA_ARGS__ , \
			epoll_fds[epfd_index], epoll_ctl_ops[op_index], \
			epoll_fds[fd_index], ev_ptr, ev_ptr->events, ev_ptr->data.fd, errno, \
			strerror(errno)); \
	} else { \
		tst_resm(TFAIL, ( "(epoll_ctl(%d,%08x,%d,%p) returned %d:%s)" ) , ##__VA_ARGS__  , \
			epoll_fds[epfd_index], epoll_ctl_ops[op_index], \
			epoll_fds[fd_index], ev_ptr, errno, strerror(errno)); \
	} \
})

#define EPOLL_CTL_TEST_PASS(msg , ...) \
({ \
	if (ev_ptr != NULL) { \
		tst_resm(TPASS, ( "(epoll_ctl(%d,%08x,%d,%p = {%08x,%08d}) returned %d:%s)" ) , ##__VA_ARGS__ , \
			epoll_fds[epfd_index], epoll_ctl_ops[op_index], \
			epoll_fds[fd_index], ev_ptr, ev_ptr->events, ev_ptr->data.fd, errno, \
			strerror(errno)); \
	} else { \
		tst_resm(TPASS, ( "(epoll_ctl(%d,%08x,%d,%p) returned %d:%s)" ) , ##__VA_ARGS__  , \
			epoll_fds[epfd_index], epoll_ctl_ops[op_index], \
			epoll_fds[fd_index], ev_ptr, errno, strerror(errno)); \
	} \
})
#else
#define EPOLL_CTL_TEST_FAIL(msg , ...) tst_resm(TFAIL, msg , ##__VA_ARGS__)
#define EPOLL_CTL_TEST_PASS(msg , ...) tst_resm(TPASS, msg , ##__VA_ARGS__)
#endif

/****************************************************************************************/

int test_epoll_ctl(int epoll_fd)
{
	int fds[] = { -1, INT_MAX };
	int epoll_fds[] = { 0, -1, 0, INT_MAX };
	int epoll_events[64];
	/* The list of operations to try AND THE ORDER THEY ARE TRIED IN */
	int epoll_ctl_ops[] =
	    { EPOLL_CTL_DEL, EPOLL_CTL_MOD, EPOLL_CTL_ADD, EPOLL_CTL_MOD,
		EPOLL_CTL_DEL, EPOLL_CTL_MOD, EPOLL_CTL_DEL, INT_MAX, -1
	};
	struct epoll_event event;
	char event_mem[sizeof(struct epoll_event) * 2];
	struct epoll_event *unaligned_event_ptr;

	/* Indices into lists */
	int index = 0;		/* multi-use index. First uses are to initialize
				   lists. Second use is to iterate over the implicit
				   list of structs to pass in */
	unsigned int epfd_index;	/* index into fd list for the epfd parameter */
	unsigned int event_index;	/* index into event list for the events field of the
					   struct epoll_event parameter */
	unsigned int fd_index;	/* index into fd list for the fd parameter */
	unsigned int op_index;	/* index into the list of operations for the op
				   parameter */
	unsigned int num_epoll_ctl_test_fails = 0;
	unsigned int num_epoll_ctl_test_calls = 0;

	/* Generate all possible combinations of events (2^6 == 64)
	   Assume we know nothing about the EPOLL event types _except_
	   that they describe bits in a set. */
	for (index = 0; index < 64; index++) {
		epoll_events[index] = ((EPOLLIN * ((index & 0x01) >> 0)) |
				       (EPOLLOUT * ((index & 0x02) >> 1)) |
				       (EPOLLPRI * ((index & 0x04) >> 2)) |
				       (EPOLLERR * ((index & 0x08) >> 3)) |
				       (EPOLLHUP * ((index & 0x10) >> 4)) |
				       (EPOLLET * ((index & 0x20) >> 5)));
	}

	/* Get a pointer to an unaligned struct epoll_event */
	{
		char *unalign_ptr = event_mem;

		unalign_ptr =
		    unalign_ptr + (((unsigned long)unalign_ptr & 1) ? 0 : 1);
		unaligned_event_ptr = (struct epoll_event *)unalign_ptr;
	}

	/* One of the fds we want to test is the valid one */
	epoll_fds[0] = epoll_fd;

	/* Test out all of the interesting combinations. This is going to
	   take a while (in compute cycles). It took less than 1 minute to
	   run on a PIII 500 without checking the results. */
	for (index = 0; index < 3; index++) {
		struct epoll_event *ev_ptr = NULL;

		switch (index) {
		case 0:	/* Pass aligned struct */
			event.data.u64 = 0;
			ev_ptr = &event;
			break;
		case 1:	/* Pass unaligned struct */
			unaligned_event_ptr->data.u64 = 0;
			ev_ptr = unaligned_event_ptr;
			break;
		case 2:
		default:	/* Pass NULL ptr */
			ev_ptr = NULL;
			break;
		}

		for (epfd_index = 0;
		     epfd_index < (sizeof(epoll_fds) / sizeof(int));
		     epfd_index++) {
			for (event_index = 0;
			     event_index < (sizeof(epoll_events) / sizeof(int));
			     event_index++) {
				for (fd_index = 0;
				     fd_index < (sizeof(fds) / sizeof(int));
				     fd_index++) {
					/* Now epoll_fd is a descriptor that references the set of
					   file descriptors we are interested in. Next we test epoll_ctl */
					for (op_index = 0;
					     op_index <
					     (sizeof(epoll_ctl_ops) /
					      sizeof(int)); op_index++) {
						int result;
						int expected_errno = 0;
						int num_errors_expected = 0;

						if (ev_ptr != NULL)
							ev_ptr->events =
							    epoll_events
							    [event_index];

						/* Perform the call itself. Put it in a protected region which
						   returns -1 in the variable result if a protection violation
						   occurs (see PROTECT_REGION_END for the result) */
						PROTECT_REGION_START errno = 0;

						/* NOTE that we are assuming that epoll will operate across
						   a fork() call such that a subsequent fork() in the parent
						   will also manipulate the same set */
						result =
						    epoll_ctl(epoll_fds
							      [epfd_index],
							      epoll_ctl_ops
							      [op_index],
							      fds[fd_index],
							      ev_ptr);

						/* We can't test errno resulting from the epoll_ctl call outside of
						   the PROTECT_REGION hence we do not have a PROTECT_REGION_END
						   here */

						/*
						   Test the results. Look for appropriate error conditions
						 */

						/* Check the epfd */
						if (epoll_fds[epfd_index] !=
						    epoll_fd) {
							/* Expect an error */
							if (epoll_fds
							    [epfd_index] == 0)
								expected_errno =
								    EINVAL;
							else	/* epfd is not a valid file descriptor since it is
								   neither epoll_fd nor stdin */
								expected_errno =
								    EBADF;
							num_errors_expected++;
						}

						switch (epoll_ctl_ops[op_index]) {
						case EPOLL_CTL_ADD:
						case EPOLL_CTL_MOD:
						case EPOLL_CTL_DEL:
							break;
						default:	/* Expect an error */
							expected_errno = EINVAL;
							num_errors_expected++;
							break;
						}

						expected_errno = EPERM;
						num_errors_expected++;

						if (ev_ptr == NULL) {
							expected_errno = EINVAL;
							num_errors_expected++;
						} else if ((ev_ptr == &event)
							   || (ev_ptr ==
							       unaligned_event_ptr))
						{
							if (ev_ptr->events == 0) {
								expected_errno =
								    EINVAL;
								num_errors_expected++;
							}

							for (index = 1;
							     index < 64;
							     index++) {
								if ((int)ev_ptr->events != epoll_events[index]) {
									expected_errno
									    =
									    EINVAL;
									num_errors_expected++;
								}
							}
						} else {
							/* Do not expect an error */
						}

						if (num_errors_expected == 0) {
							/* We did not expect an error */
							if (result == 0) {
								/* We didn't get an error. Think of this as RES_PASS_RETV_MAT_ERRNO_IGN */
								return RES_PASS;
							} else if (result == -1) {	/* The return value is -1, so it's not bad */
								return
								    RES_FAIL_RETV_MIS_ERRNO_IGN;
							} else {
								return
								    RES_FAIL_RETV_BAD_ERRNO_IGN;
							}
						} else if (num_errors_expected
							   == 1) {
							/* We expected an error */
							if (result == 0) {
								return RES_FAIL_RETV_MIS_ERRNO_IGN;	/* Unexpected success */
							} else if (result == -1) {
								/* We got an error. Check errno */
								if (errno ==
								    expected_errno)
								{
									return RES_PASS;	/* think of this as RETV_MAT_ERRNO_MAT */
								} else {
									return
									    RES_FAIL_RETV_MAT_ERRNO_MIS;
								}
							} else {
								/* We got a bad return code! Interpret this as
								   getting an error and check errno. */
								if (errno ==
								    expected_errno)
									return
									    RES_FAIL_RETV_BAD_ERRNO_MAT;
								else
									return
									    RES_FAIL_RETV_BAD_ERRNO_MIS;
							}
						} else if (num_errors_expected >
							   1) {
							/* We expected multiple errors */
							if (result == 0) {
								return RES_FAIL_RETV_MIS_ERRNO_IGN;	/* Unexpected success */
							} else if (result == -1) {
								/* We got an error. Check errno */
								if (errno ==
								    expected_errno)
								{
									return RES_PASS;	/* think of this as RETV_MAT_ERRNO_MAT */
								} else {
									/* Ignore errno because the desired value is unknowable
									   without looking at the structure of the code. */
									return
									    RES_PASS_RETV_MAT_ERRNO_IGN;
								}
							} else {
								/* We got a bad return code! Interpret this as
								   getting an error and check errno. */
								if (errno ==
								    expected_errno)
									/* Don't Ignore errno because the desired value
									   happened to match what we expected. */
									return
									    RES_FAIL_RETV_BAD_ERRNO_MAT;
								else
									/* Ignore errno because the desired value is unknowable
									   without looking at the structure of the code. */
									return
									    RES_FAIL_RETV_BAD_ERRNO_IGN;
							}
						}

						/* All "return"s between PROTECT_REGION_BEGIN
						   and PROTECT_REGION_END place their value in
						   the result parameter. If the region caused
						   a protection violation (segfault or otherwise)
						   then the result is set to the second parameter's
						   value (-1 in this case). */
						PROTECT_REGION_END(result, -1);

						/* Count the number of tests run */
						num_epoll_ctl_test_calls++;

						/* Now test the result */
						if (!((result == RES_PASS)
						      || (result ==
							  RES_PASS_RETV_MAT_ERRNO_IGN)))
						{
							if (result >
							   (int)(sizeof(result_strings) /
							     sizeof(const char
								    *))) {
								/* Returned a result which has no corresponding text description */
								EPOLL_CTL_TEST_FAIL
								    ("FIXME FIX ME BUG in Test Program itself!");
							} else {
								if (result == -1)	/* Segfault during epoll_ctl call */
									EPOLL_CTL_TEST_FAIL
									    ("Test arguments caused abnormal exit.");
								else	/* The 'normal' failure */
									EPOLL_CTL_TEST_FAIL
									    ((result_strings[result]));
							}
							num_epoll_ctl_test_fails++;
#ifdef DEBUG
						} else	/* The call of epoll_ctl behaved as expected */
							EPOLL_CTL_TEST_PASS((result_strings[result]));
#else
						}
#endif
					}
				}
			}
		}
		close(epoll_fd);
	}

	tst_resm(TINFO,
		 "Summary: Of %d tests, epoll_ctl failed %d (%3.0f%% passed).",
		 num_epoll_ctl_test_calls, num_epoll_ctl_test_fails,
		 ((float)(num_epoll_ctl_test_calls - num_epoll_ctl_test_fails) *
		  100.0f / (float)num_epoll_ctl_test_calls));
	return (num_epoll_ctl_test_fails / num_epoll_ctl_test_calls);
}

int main(void)
{
	int epoll_fd;
	struct timeval tv;
	int last_result;

	tst_resm(TINFO, "testing if epoll() system call works");

	/* Get the current time */
	if (gettimeofday(&tv, NULL) != 0) {
		tst_brkm(TBROK | TERRNO, NULL, "gettimeofday failed");
	} else {
		tst_resm(TINFO, "gettimeofday() works");
	}

	/* Set up RNG */
	srand(tv.tv_usec);
	tst_resm(TINFO,
		 "random number seeded with gettimeofday() [seed = %ld] works",
		 tv.tv_usec);

	tst_resm(TINFO, "Testing epoll_create");
	/* Testing epoll_create with some different sizes */
	last_result = PROTECT_FUNC(test_epoll_create, -1, NUM_RAND_ATTEMPTS);
	if (last_result != 0) {
		/* create test(s) failed */
	}

	/* Create an epoll_fd for testing epoll_ctl */
	epoll_fd = epoll_create(BACKING_STORE_SIZE_HINT);
	if (epoll_fd < 0) {
		tst_brkm(TFAIL | TERRNO, NULL, "epoll_create failed");
	}

	tst_resm(TINFO, "Testing epoll_ctl");
	last_result = PROTECT_FUNC(test_epoll_ctl, -1, epoll_fd);
	if (last_result != 0) {
		/* ctl test(s) failed */
	}

	tst_exit();
}

#else

int main(void)
{
	tst_brkm(TCONF, NULL, "No epoll support found.");
}

#endif
