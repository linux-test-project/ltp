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
#include "lapi/syscalls.h"
#include "libswapon.h"

static void setup(void);
static void cleanup(void);

char *TCID = "swapon01";
int TST_TOTAL = 1;

static long fs_type;

static void verify_swapon(void)
{
	TEST(ltp_syscall(__NR_swapon, "./swapfile01", 0));

	if (TEST_RETURN == -1) {
		if (fs_type == TST_BTRFS_MAGIC && errno == EINVAL) {
			tst_brkm(TCONF, cleanup,
			         "Swapfile on BTRFS not implemeted");
			return;
		}
		tst_resm(TFAIL | TTERRNO, "Failed to turn on swapfile");
	} else {
		tst_resm(TPASS, "Succeeded to turn on swapfile");
		/*we need to turn this swap file off for -i option */
		if (ltp_syscall(__NR_swapoff, "./swapfile01") != 0) {
			tst_brkm(TBROK, cleanup, "Failed to turn off swapfile,"
			         " system reboot after execution of LTP "
				 "test suite is recommended.");
		}
	}
}

int main(int ac, char **av)
{

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		verify_swapon();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	is_swap_supported(cleanup, "./tstswap");

	make_swapfile(cleanup, "swapfile01", 0);
}

static void cleanup(void)
{
	tst_rmdir();
}
