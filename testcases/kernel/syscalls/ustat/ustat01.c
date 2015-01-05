/*
 * Copyright (c) Wipro Technologies Ltd, 2003.  All Rights Reserved.
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
 * Check that ustat() succeeds given correct parameters.
 */

#include <unistd.h>
#include <ustat.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

char *TCID = "ustat01";
int TST_TOTAL = 1;

static dev_t dev_num;
static struct ustat ubuf;

int main(int argc, char *argv[])
{
	int lc, i;
	const char *msg;

	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(ustat(dev_num, &ubuf));

			if (TEST_RETURN == -1 && TEST_ERRNO == ENOSYS)
				tst_brkm(TCONF, cleanup, "ustat not supported");

			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL, "ustat(2) failed and set"
					 "the errno to %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			} else {
				tst_resm(TPASS, "ustat(2) passed");
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct stat buf;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Find a valid device number */
	SAFE_STAT(cleanup, "/", &buf);

	dev_num = buf.st_dev;
}

static void cleanup(void)
{
	TEST_CLEANUP;
}
