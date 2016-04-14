/*
 * Copyright (c) International Business Machines  Corp., 2003.
 * Copyright (c) 2012 Wanlong Gao <gaowanlong@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/*
 *	This is a test for a glibc bug for the clone(2) system call.
 */

#if defined UCLINUX && !__THROW
/* workaround for libc bug */
#define __THROW
#endif

#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include "test.h"
#include "clone_platform.h"

#define TRUE 1
#define FALSE 0

static void setup();
static int do_child();

char *TCID = "clone07";
int TST_TOTAL = 1;

static void sigsegv_handler(int);
static void sigusr2_handler(int);
static int child_pid;
static int fail = FALSE;

int main(int ac, char **av)
{

	int lc, status;
	void *child_stack;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		child_stack = malloc(CHILD_STACK_SIZE);
		if (child_stack == NULL)
			tst_brkm(TBROK, NULL,
				 "Cannot allocate stack for child");

		child_pid = ltp_clone(SIGCHLD, do_child, NULL,
				      CHILD_STACK_SIZE, child_stack);

		if (child_pid < 0)
			tst_brkm(TBROK | TERRNO, NULL, "clone failed");

		if ((wait(&status)) == -1)
			tst_brkm(TBROK | TERRNO, NULL,
				 "wait failed, status: %d", status);

		free(child_stack);
	}

	if (fail == FALSE)
		tst_resm(TPASS,
			 "Use of return() in child did not cause SIGSEGV");
	else
		tst_resm(TFAIL, "Use of return() in child caused SIGSEGV");

	tst_exit();
}

static void setup(void)
{
	struct sigaction def_act;
	struct sigaction act;

	TEST_PAUSE;

	act.sa_handler = sigsegv_handler;
	act.sa_flags = SA_RESTART;
	sigemptyset(&act.sa_mask);
	if ((sigaction(SIGSEGV, &act, NULL)) == -1)
		tst_resm(TWARN | TERRNO,
			 "sigaction() for SIGSEGV failed in test_setup()");

	/* Setup signal handler for SIGUSR2 */
	def_act.sa_handler = sigusr2_handler;
	def_act.sa_flags = SA_RESTART | SA_RESETHAND;
	sigemptyset(&def_act.sa_mask);

	if ((sigaction(SIGUSR2, &def_act, NULL)) == -1)
		tst_resm(TWARN | TERRNO,
			 "sigaction() for SIGUSR2 failed in test_setup()");
}

static int do_child(void)
{
	return 0;
}

static void sigsegv_handler(int sig)
{
	if (child_pid == 0) {
		kill(getppid(), SIGUSR2);
		_exit(42);
	}
}

/* sig_default_handler() - Default handler for parent */
static void sigusr2_handler(int sig)
{
	if (child_pid != 0)
		fail = TRUE;
}
