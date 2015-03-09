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
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */
 /*
  *     Verify that,
  *      clone(2) returns -1 and sets errno to EINVAL if
  *     child stack is set to a zero value(NULL)
  */

#if defined UCLINUX && !__THROW
/* workaround for libc bug */
#define __THROW
#endif

#include <sched.h>
#include <errno.h>
#include <sys/wait.h>
#include "test.h"
#include "clone_platform.h"

static void cleanup(void);
static void setup(void);
static int child_fn();

static void *child_stack;

static struct test_case_t {
	int (*child_fn) ();
	void **child_stack;
	int exp_errno;
	char err_desc[10];
} test_cases[] = {
	{
child_fn, NULL, EINVAL, "EINVAL"},};

char *TCID = "clone04";
int TST_TOTAL = sizeof(test_cases) / sizeof(test_cases[0]);

int main(int ac, char **av)
{
	int lc, ind;
	void *test_stack;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (ind = 0; ind < TST_TOTAL; ind++) {
			if (test_cases[ind].child_stack == NULL) {
				test_stack = NULL;
			} else if (*test_cases[ind].child_stack == NULL) {
				tst_resm(TWARN, "Can not allocate stack for"
					 "child, skipping test case");
				continue;
			} else {
				test_stack = child_stack;
			}

			TEST(ltp_clone(0, test_cases[ind].child_fn, NULL,
				       CHILD_STACK_SIZE, test_stack));

			if ((TEST_RETURN == -1) &&
			    (TEST_ERRNO == test_cases[ind].exp_errno)) {
				tst_resm(TPASS, "expected failure; Got %s",
					 test_cases[ind].err_desc);
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "Call failed to produce expected error; "
					 "expected errno %d and result -1; got result %ld",
					 test_cases[ind].exp_errno,
					 TEST_RETURN);
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	child_stack = malloc(CHILD_STACK_SIZE);
}

static void cleanup(void)
{
	free(child_stack);
}

static int child_fn(void)
{
	exit(1);
}
