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
 * Check that swapoff() succeeds.
 */

#include <unistd.h>
#include "test.h"
#include "usctest.h"
#include <errno.h>
#include <stdlib.h>
#include "config.h"
#include "linux_syscall_numbers.h"
#include "tst_fs_type.h"
#include "swaponoff.h"

static void setup(void);
static void cleanup(void);

char *TCID = "swapoff01";
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

		if (ltp_syscall(__NR_swapon, "./swapfile01", 0) != 0) {
			tst_resm(TWARN, "Failed to turn on the swap file"
				 ", skipping test iteration");
			continue;
		}

		TEST(ltp_syscall(__NR_swapoff, "./swapfile01"));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "swapoff(2) Failed to turn off"
				 " swapfile. System reboot after execution"
				 " of LTP test suite is recommended.");
			tst_resm(TWARN, "It is recommended not to run"
				 " swapon01 and swapon02");
		} else {
			tst_resm(TPASS, "swapoff(2) passed and turned off"
				 " swapfile.");
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	long type;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	TEST_PAUSE;

	tst_tmpdir();

	switch ((type = tst_fs_type(cleanup, "."))) {
	case TST_NFS_MAGIC:
	case TST_TMPFS_MAGIC:
		tst_brkm(TCONF, cleanup,
			 "Cannot do swapoff on a file on %s filesystem",
			 tst_fs_type_name(type));
	break;
	}

	if (!tst_cwd_has_free(65536)) {
		tst_brkm(TBROK, cleanup,
			 "Insufficient disk space to create swap file");
	}

	/*create file */
	if (tst_fill_file("swapfile01", 0x00, 1024, 65536))
		tst_brkm(TBROK, cleanup, "Failed to create file for swap");

	/* make above file a swap file */
	if (system("mkswap swapfile01 > tmpfile 2>&1") != 0) {
		tst_brkm(TBROK, cleanup, "Failed to make swapfile");
	}
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
