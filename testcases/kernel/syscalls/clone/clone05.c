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
 *	Call clone() with CLONE_VFORK flag set. verify that
 *	execution of parent is suspended until child finishes
 *	Execute system call with CLONE_VM & CLONE_VFORK flags
 */

#if defined UCLINUX && !__THROW
/* workaround for libc bug */
#define __THROW
#endif

#define _GNU_SOURCE

#include <errno.h>
#include <sched.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"
#include "clone_platform.h"

#define FLAG (CLONE_VM|CLONE_VFORK)

static void setup(void);
static void cleanup(void);
static int child_fn();

static int parent_variable;

char *TCID = "clone05";
int TST_TOTAL = 1;

int main(int ac, char **av)
{

	int lc, status;
	char *msg;
	void *child_stack;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	child_stack = malloc(CHILD_STACK_SIZE);
	if (child_stack == NULL)
		tst_brkm(TBROK, cleanup, "Cannot allocate stack for child");

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		TEST(ltp_clone(FLAG | SIGCHLD, child_fn, NULL, CHILD_STACK_SIZE,
			       child_stack));

		if ((TEST_RETURN != -1) && (parent_variable))
			tst_resm(TPASS, "Test Passed");
		else
			tst_resm(TFAIL, "Test Failed");

		if ((wait(&status)) == -1)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "wait failed, status: %d", status);

		/* Reset parent_variable */
		parent_variable = 0;
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
	TEST_CLEANUP;
}

static int child_fn()
{
	/*
	 * Sleep for a second, to ensure that child does not exit
	 * immediately
	 */
	sleep(1);
	parent_variable = 1;
	exit(1);
}
