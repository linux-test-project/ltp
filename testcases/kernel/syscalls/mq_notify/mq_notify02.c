/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
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
 * with this program.
 */
/*
 * ALGORITHM
 *	test 1:
 *		sevp->sigev_notify = -1, EINVAL should be returned.
 *	test 2:
 *		sevp->sigev_notify = SIGEV_SIGNAL and sevp->sigev_signo > _NSG,
 *		EINVAL should be returned.
 */

#include <errno.h>
#include <mqueue.h>
#include "test.h"

char *TCID = "mq_notify02";
static void setup(void);
static void cleanup(void);

static struct test_case_t {
	struct sigevent sevp;
	int exp_errno;
} test_cases[] = {
	{{.sigev_notify = -1}, EINVAL},
	{{.sigev_notify = SIGEV_SIGNAL, .sigev_signo = _NSIG+1}, EINVAL},
};

int TST_TOTAL = ARRAY_SIZE(test_cases);
static void mq_notify_verify(struct test_case_t *);

int main(int argc, char **argv)
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			mq_notify_verify(&test_cases[i]);
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void mq_notify_verify(struct test_case_t *test)
{
	TEST(mq_notify(0, &(test->sevp)));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "mq_notify() succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_errno) {
		tst_resm(TPASS | TTERRNO, "mq_notify failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "mq_notify failed unexpectedly; expected: %d - %s",
			 test->exp_errno, strerror(test->exp_errno));
	}
}

static void cleanup(void)
{
}
