/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 * 	sigaction02.c
 *
 * DESCRIPTION
 * 	Testcase to check the basic errnos set by the sigaction(2) syscall.
 *
 * ALGORITHM
 *	1. Pass an invalid signal as the "sig" parameter, and expect EINVAL.
 *	2. Attempt to catch the SIGKILL, and expect EINVAL.
 *	3. Attempt to catch the SIGSTOP, and expect EINVAL.
 *	4. Pass an invalid address as the "act" parameter, expect an EFAULT.
 *	5. Pass an invalid address as the "oact" parameter, and expect EFAULT.
 *
 * USAGE
 *	sigaction02
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	Tests #4 and #5 will fail as long as the glibc implementation
 *	of sigaction() is not fixed.  The glibc wrapper around of sigaction()
 *	doesn't handle the invalid addresses of the "act" and "oact" parameters
 *	correctly.  If an invalid address is passed, glibc dumps core.
 *	Temporarily, tests 4 and 5 are put inside "#ifdef GLIBC_SIGACTION_BUG"
 *	in order to skip these tests.  This should be removed from the Makefile
 *	and this program when the glibc bug gets fixed.
 *
 *	This test doesn't follow the correct LTP format - PLEASE FIX!
 */
#define DEBUG 0

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "test.h"

#define SIGBAD 9999

void setup();
void cleanup();

char *TCID = "sigaction02";
int TST_TOTAL = 1;

volatile sig_atomic_t testcase_no;

/*
 * handler()
 *	A dummy signal handler for attempting to catch signals.
 */
void handler(int sig)
{
	if (DEBUG)
		tst_resm(TINFO, "Inside signal handler. Got signal: %d", sig);
	return;
}

/*
 * set_handler()
 * 	Establish a signal handler for "sig" with the specified flags and
 * 	signal to mask while the handler executes.
 * Returns
 *	0 on success, errno on failure
 */
int set_handler(int sig, int sig_to_mask, int flag)
{
	struct sigaction sa;
	int err;

	if (flag == 0) {
		sa.sa_sigaction = (void *)handler;
		sa.sa_flags = SA_NOMASK;
		sigemptyset(&sa.sa_mask);
		sigaddset(&sa.sa_mask, sig_to_mask);
		err = sigaction(sig, &sa, NULL);
	} else if (flag == 1) {
		err = sigaction(sig, (void *)-1, NULL);
	} else if (flag == 2) {
		err = sigaction(sig, NULL, (void *)-1);
	} else
		err = -1;

	if (err == 0)
		return 0;
	else
		return errno;
}

int main(int ac, char **av)
{
	int ret;

	tst_parse_opts(ac, av, NULL, NULL);
//test1:
	testcase_no = 1;

	if (DEBUG)
		tst_resm(TINFO, "Enter test %d: set handler for SIGKILL",
			 testcase_no);
	if ((ret = set_handler(SIGKILL, 0, 0)) == 0) {
		tst_resm(TFAIL, "sigaction() succeeded, should have failed");
	}
	if (ret != EINVAL) {
		tst_resm(TFAIL, "sigaction set incorrect errno. Expected "
			 "EINVAL, got: %d", ret);
	} else {
		tst_resm(TPASS, "call failed with expected EINVAL error");
	}

//test2:
	testcase_no++;

	if (DEBUG)
		tst_resm(TINFO, "Enter test %d: set handler for SIGSTOP",
			 testcase_no);
	if ((ret = set_handler(SIGSTOP, 0, 0)) == 0) {
		tst_resm(TFAIL, "sigaction() succeeded, should have failed");
	}
	if (ret != EINVAL) {
		tst_resm(TFAIL, "sigaction set incorrect errno. Expected "
			 "EINVAL, got: %d", ret);
	} else {
		tst_resm(TPASS, "call failed with expected EINVAL error");
	}

//test3:
	testcase_no++;
	if (DEBUG)
		tst_resm(TINFO, "Enter test %d: set handler for bad "
			 "signal number", testcase_no);
	if ((ret = set_handler(SIGBAD, 0, 0)) == 0) {
		tst_resm(TFAIL, "sigaction() succeeded, should have failed");
	}
	if (ret != EINVAL) {
		tst_resm(TFAIL, "sigaction set incorrect errno. Expected "
			 "EINVAL, got: %d", ret);
	} else {
		tst_resm(TPASS, "call failed with expected EINVAL error");
	}

#ifndef GLIBC_SIGACTION_BUG

//test4:
	testcase_no++;
	if (DEBUG)
		tst_resm(TINFO, "Enter test %d: set handler with "
			 "bad \"act\" param", testcase_no);
	if ((ret = set_handler(SIGUSR1, 0, 1)) == 0) {
		tst_resm(TFAIL, "sigaction() succeeded, should have failed");
	}
	if (ret != EFAULT) {
		tst_resm(TFAIL, "sigaction set incorrect errno. Expected "
			 "EFAULT, got: %d", ret);
	} else {
		tst_resm(TPASS, "call failed with expected EFAULT error");
	}

//test5:
	testcase_no++;
	if (DEBUG)
		tst_resm(TINFO, "Enter test %d: set handler with "
			 "bad \"oact\" param", testcase_no);
	if ((ret = set_handler(SIGUSR1, 0, 2)) == 0) {
		tst_resm(TFAIL, "sigaction() succeeded, should have failed");
	}
	if (ret != EFAULT) {
		tst_resm(TFAIL, "sigaction set incorrect errno. Expected "
			 "EFAULT, got: %d", ret);
	} else {
		tst_resm(TPASS, "call failed with expected EFAULT error");
	}
#endif /* GLIBC_SIGACTION_BUG */

	tst_exit();

}
