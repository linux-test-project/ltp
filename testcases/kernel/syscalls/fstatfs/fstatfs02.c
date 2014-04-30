/*
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
 */

/*
 * DESCRIPTION
 *	Testcase to check fstatfs() sets errno correctly.
 */

#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

static void setup(void);
static void cleanup(void);

char *TCID = "fstatfs02";

static int exp_enos[] = { EBADF, EFAULT, 0 };

static struct statfs buf;

static struct test_case_t {
	int fd;
	struct statfs *sbuf;
	int error;
} TC[] = {
	/* EBADF - fd is invalid */
	{
	-1, &buf, EBADF},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	    /* EFAULT - address for buf is invalid */
	{
	1, (void *)-1, EFAULT}
#endif
};

int TST_TOTAL = sizeof(TC) / sizeof(*TC);

int main(int ac, char **av)
{
	int lc;
	int i;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(fstatfs(TC[i].fd, TC[i].sbuf));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].error);
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

	tst_tmpdir();
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
