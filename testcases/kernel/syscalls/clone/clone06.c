/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
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
 *	Test to verify inheritance of environment variables by child.
 */

#if defined UCLINUX && !__THROW
/* workaround for libc bug */
#define __THROW
#endif

#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "test.h"
#include "clone_platform.h"

#define MAX_LINE_LENGTH 256

static void setup(void);
static void cleanup(void);
static int child_environ();

static int pfd[2];

char *TCID = "clone06";
int TST_TOTAL = 1;

int main(int ac, char **av)
{

	int lc, status;
	void *child_stack;
	char *parent_env;
	char buff[MAX_LINE_LENGTH];

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	child_stack = malloc(CHILD_STACK_SIZE);
	if (child_stack == NULL)
		tst_brkm(TBROK, cleanup, "Cannot allocate stack for child");

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if ((pipe(pfd)) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "pipe() failed");

		TEST(ltp_clone(SIGCHLD, child_environ, NULL, CHILD_STACK_SIZE,
			       child_stack));

		if (TEST_RETURN == -1)
			tst_brkm(TFAIL | TTERRNO, cleanup, "clone failed");

		/* close write end from parent */
		if ((close(pfd[1])) == -1)
			tst_resm(TWARN | TERRNO, "close(pfd[1]) failed");

		/* Read env var from read end */
		if ((read(pfd[0], buff, sizeof(buff))) == -1)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "read from pipe failed");

		/* Close read end from parent */
		if ((close(pfd[0])) == -1)
			tst_resm(TWARN | TERRNO, "close(pfd[0]) failed");

		parent_env = getenv("TERM") ? : "";

		if ((strcmp(buff, parent_env)) == 0)
			tst_resm(TPASS, "Test Passed");
		else
			tst_resm(TFAIL, "Test Failed");

		if ((wait(&status)) == -1)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "wait failed, status: %d", status);
	}

	free(child_stack);

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

static void cleanup(void)
{
}

/*
 *	Function executed by child.
 *	Gets the value for environment variable,TERM &
 *	writes it to  a pipe.
 */
static int child_environ(void)
{

	char var[MAX_LINE_LENGTH];

	/* Close read end from child */
	if ((close(pfd[0])) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "close(pfd[0]) failed");

	if ((sprintf(var, "%s", getenv("TERM") ? : "")) < 0)
		tst_resm(TWARN | TERRNO, "sprintf() failed");

	if ((write(pfd[1], var, MAX_LINE_LENGTH)) == -1)
		tst_resm(TWARN | TERRNO, "write to pipe failed");

	/* Close write end of pipe from child */
	if ((close(pfd[1])) == -1)
		tst_resm(TWARN | TERRNO, "close(pfd[1]) failed");

	exit(0);
}
