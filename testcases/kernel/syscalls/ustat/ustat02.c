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
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "test.h"
#include "safe_macros.h"

char *TCID = "ustat02";

#ifdef HAVE_USTAT
# ifdef HAVE_SYS_USTAT_H
#  include <sys/ustat.h>
# endif

static void setup(void);

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

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(ustat(*tc[i].dev, tc[i].buf));

			if (TEST_RETURN == -1 && TEST_ERRNO == ENOSYS)
				tst_brkm(TCONF, NULL, "ustat not supported");

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
		}
	}

	tst_exit();
}

static void setup(void)
{
	struct stat buf;

	tst_sig(NOFORK, DEF_HANDLER, NULL);

	TEST_PAUSE;

	/* Find a valid device number */
	SAFE_STAT(NULL, "/", &buf);

	root_dev = buf.st_dev;
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "system doesn't have ustat() support");
}
#endif
