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
#include <pthread.h>
#include <tst_safe_pthread.h>

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

static void* do_child_thread(void* arg);
static void* do_master_thread(void* arg);

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
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
		 * Creates two threads and tries to do setsid()
		 */
		pthread_t master_tid;

		SAFE_PTHREAD_CREATE(&master_tid, NULL, do_master_thread, NULL);

		SAFE_PTHREAD_JOIN(master_tid, NULL);
	}
	cleanup();
	tst_exit();

}

static void* do_master_thread(void* arg)
{
	pthread_t child_tid;

	if (setpgid(0, 0) < 0) {
		tst_resm(TFAIL, "master thread: setpgid(0,0) failed with error: %s", strerror(errno));
		pthread_exit(NULL);
	}
	else
	{
		tst_resm(TINFO, "master thread: setpgid(0,0) success");
	}

	SAFE_PTHREAD_CREATE(&child_tid, NULL, do_child_thread, NULL);

	SAFE_PTHREAD_JOIN(child_tid, NULL);

	pthread_exit(NULL);

}


static void* do_child_thread(void* arg)
{
	int retval;

	/* Since thread is being used , need to replace getppid() with getpid() */
	retval = setpgid(0, getpid());

	if (retval < 0) {
		tst_resm(TFAIL, "setpgid failed, errno :%d", errno);
	}
	else
	{
		tst_resm(TINFO, "setpgid for child thread success");
	}

	retval = setsid();

	if (errno == EPERM) {
		tst_resm(TPASS, "setsid SUCCESS to set "
				"errno to EPERM");
	} else {
		tst_resm(TFAIL, "setsid failed, expected %d,"
				"return %d", -1, errno);
	}

	pthread_exit(NULL);
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
