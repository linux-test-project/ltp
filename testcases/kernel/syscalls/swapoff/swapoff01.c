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
#include <errno.h>
#include <stdlib.h>
#include "config.h"
#include "lapi/syscalls.h"
#include "../swapon/libswapon.h"

static void setup(void);
static void cleanup(void);
static void verify_swapoff(void);

char *TCID = "swapoff01";
int TST_TOTAL = 1;

static long fs_type;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		verify_swapoff();
	}

	cleanup();
	tst_exit();
}

static void verify_swapoff(void)
{
	if (ltp_syscall(__NR_swapon, "./swapfile01", 0) != 0) {
		if (fs_type == TST_BTRFS_MAGIC && errno == EINVAL) {
			tst_brkm(TCONF, cleanup,
			         "Swapfiles on BTRFS are not implemented");
		}

		tst_resm(TBROK, "Failed to turn on the swap file"
			 ", skipping test iteration");
		return;
	}

	TEST(ltp_syscall(__NR_swapoff, "./swapfile01"));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "Failed to turn off swapfile,"
		         " system reboot after execution of LTP "
			 "test suite is recommended.");
	} else {
		tst_resm(TPASS, "Succeeded to turn off swapfile");
	}
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	is_swap_supported(cleanup, "./tstswap");

	if (!tst_fs_has_free(NULL, ".", 64, TST_MB)) {
		tst_brkm(TBROK, cleanup,
			 "Insufficient disk space to create swap file");
	}

	if (tst_fill_file("swapfile01", 0x00, 1024, 65536))
		tst_brkm(TBROK, cleanup, "Failed to create file for swap");

	if (system("mkswap swapfile01 > tmpfile 2>&1") != 0)
		tst_brkm(TBROK, cleanup, "Failed to make swapfile");
}

static void cleanup(void)
{
	tst_rmdir();
}
