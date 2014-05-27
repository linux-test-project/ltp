/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
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
 * Test whether ustat(2) system call returns appropriate error number for
 * invalid dev_t parameter and for bad address paramater.
 */

#include <unistd.h>
#include <ustat.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

char *TCID = "ustat02";

static int exp_enos[] = { EINVAL, EFAULT, 0 };

static dev_t invalid_dev = -1;
static dev_t root_dev;
struct ustat ubuf;

static struct test_case_t {
	char *err_desc;
	int exp_errno;
	char *exp_errval;
	dev_t *dev;
	struct ustat *buf;
} tc[] = {
	{"Invalid parameter", EINVAL, "EINVAL", &invalid_dev, &ubuf},
#ifndef UCLINUX
	{"Bad address", EFAULT, "EFAULT", &root_dev, (void*)-1}
#endif
};

int TST_TOTAL = ARRAY_SIZE(tc);

int main(int ac, char **av)
{

	int lc, i;
	const char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(ustat(*tc[i].dev, tc[i].buf));

			if ((TEST_RETURN == -1)
			    && (TEST_ERRNO == tc[i].exp_errno)) {
				tst_resm(TPASS,
					 "ustat(2) expected failure;"
					 " Got errno - %s : %s",
					 tc[i].exp_errval, tc[i].err_desc);
			} else {
				tst_resm(TFAIL | TTERRNO,
				         "ustat(2) failed to produce"
					 " expected error; %d, errno"
					 ": %s",
					 tc[i].exp_errno, tc[i].exp_errval);
			}

			TEST_ERROR_LOG(TEST_ERRNO);
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct stat buf;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	/* Find a valid device number */
	SAFE_STAT(cleanup, "/", &buf);

	root_dev = buf.st_dev;
}

static void cleanup(void)
{
	TEST_CLEANUP;
}
