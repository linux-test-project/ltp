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
  * Checks that swapon() succeds with swapfile.
  */

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "test.h"
#include "usctest.h"
#include "config.h"
#include "linux_syscall_numbers.h"
#include "swaponoff.h"
#include "libswapon.h"

static void setup(void);
static void cleanup(void);

char *TCID = "swapon01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{

	int lc;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(ltp_syscall(__NR_swapon, "./swapfile01", 0));

		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "swapon(2) Failed to turn on"
				 " swapfile.");
		} else {
			tst_resm(TPASS, "swapon(2) passed and turned on"
				 " swapfile");
			/*we need to turn this swap file off for -i option */
			if (ltp_syscall(__NR_swapoff, "./swapfile01") != 0) {
				tst_brkm(TBROK, cleanup, "Failed to turn off"
					 " swapfile. system"
					 " reboot after"
					 " execution of LTP"
					 " test suite is" " recommended.");
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	TEST_PAUSE;

	tst_tmpdir();

	if (tst_is_cwd_tmpfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a tmpfs filesystem");
	}

	if (tst_is_cwd_nfs()) {
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapon on a file located on a nfs filesystem");
	}

	make_swapfile(cleanup, "swapfile01");
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
