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
 * 	setsid01.c
 *
 * DESCRIPTION
 * 	Test to check the error and trivial conditions in setsid system call
 *
 * USAGE
 * 	setsid01
 *
 * RESTRICTIONS
 * 	This test doesn't follow good LTP format - PLEASE FIX!
 */
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"

#define INVAL_FLAG	-1
#define USER2		301
#define INVAL_MAXGRP	NGROUPS_MAX + 1
#define INVAL_USER	999999
#define INVAL_PID	999999

char *TCID = "setsid01";
int TST_TOTAL = 1;

#ifdef UCLINUX
static char *argv0;
#endif

void do_child_1(void);
void do_child_2(void);
void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int pid;
	int fail = 0;
	int ret, status;
	int exno = 0;

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);
#ifdef UCLINUX
	argv0 = av[0];

	maybe_run_child(&do_child_1, "n", 1);
	maybe_run_child(&do_child_2, "n", 2);
#endif

	/*
	 * perform global setup for the test
	 */
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		/*
		 * When the process group having forked of a child
		 * and then it attached itself to another process
		 * group and tries to setsid
		 */
		pid = FORK_OR_VFORK();

		if (pid == 0) {
			if ((pid = FORK_OR_VFORK()) == -1) {
				tst_resm(TFAIL, "Fork failed");

			}
			if (pid == 0) {
#ifdef UCLINUX
				if (self_exec(argv0, "n", 1) < 0) {
					tst_resm(TFAIL, "self_exec failed");

				}
#else
				do_child_1();
#endif
			} else {
				if (setpgid(0, 0) < 0) {
					tst_resm(TFAIL,
						 "setpgid(parent) failed: %s",
						 strerror(errno));
					fail = 1;
				}

				if ((ret = wait(&status)) > 0) {
					if (status != 0) {
						tst_resm(TFAIL,
							 "Test {%d} exited "
							 "status 0x%0x (wanted 0x0)",
							 ret, status);
						fail = 1;
					}
				}
			}
			exit(0);
		} else {
			if ((ret = wait(&status)) > 0) {
				if (status != 0) {
					tst_resm(TFAIL, "Test {%d} exited "
						 "status 0x%0x (wanted 0x0)",
						 ret, status);
					fail = 1;
				}
			}
		}

		if (!(fail || exno)) {
			tst_resm(TPASS, "all misc tests passed");
		}
	}
	cleanup();
	tst_exit();

}

/*
 * do_child_1()
 */
void do_child_1(void)
{
	int exno = 0;
	int retval, ret, status;
	int pid;

	sleep(1);

	if (setpgid(0, 0) < 0) {
		tst_resm(TFAIL, "setpgid(0,0) failed: %s", strerror(errno));
		exno = 1;
	}

	if ((pid = FORK_OR_VFORK()) == -1) {
		tst_brkm(TFAIL, NULL, "Fork failed");
	}
	if (pid == 0) {
#ifdef UCLINUX
		if (self_exec(argv0, "n", 2) < 0) {
			tst_brkm(TFAIL, NULL, "self_exec failed");
		}
#else
		do_child_2();
#endif
	} else {
		retval = setpgid(0, getppid());
		if (retval < 0) {
			tst_resm(TFAIL, "setpgid failed, errno :%d", errno);
			exno = 2;
		}

		retval = setsid();

		if (errno == EPERM) {
			tst_resm(TPASS, "setsid SUCCESS to set "
				 "errno to EPERM");
		} else {
			tst_resm(TFAIL, "setsid failed, expected %d,"
				 "return %d", -1, errno);
			exno = 3;
		}
		kill(pid, SIGKILL);
		if ((ret = wait(&status)) > 0) {
			if (status != 9) {
				tst_resm(TFAIL,
					 "Test {%d} exited status 0x%-x (wanted 0x9)",
					 ret, status);
				exno = 4;
			}
		}
	}
	exit(exno);
}

/*
 * do_child_2()
 */
void do_child_2(void)
{
	for (;;) ;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit
 */
void cleanup(void)
{

}
