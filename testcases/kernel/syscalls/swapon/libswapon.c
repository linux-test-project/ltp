/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: Stanislav Kholmanskikh <stanislav.kholmanskikh@oracle.com>
 *
 */

#include <errno.h>
#include "lapi/syscalls.h"
#include "test.h"
#include "libswapon.h"

/*
 * Make a swap file
 */
int make_swapfile(void (cleanup)(void), const char *swapfile, int safe)
{
	if (!tst_fs_has_free(NULL, ".", sysconf(_SC_PAGESIZE) * 10,
	    TST_BYTES)) {
		tst_brkm(TBROK, cleanup,
			"Insufficient disk space to create swap file");
	}

	/* create file */
	if (tst_fill_file(swapfile, 0,
			sysconf(_SC_PAGESIZE), 10) != 0) {
		tst_brkm(TBROK, cleanup, "Failed to create swapfile");
	}

	/* make the file swapfile */
	const char *argv[2 + 1];
	argv[0] = "mkswap";
	argv[1] = swapfile;
	argv[2] = NULL;

	return tst_cmd(cleanup, argv, "/dev/null", "/dev/null", safe);
}

/*
 * Check swapon/swapoff support status of filesystems or files
 * we are testing on.
 */
void is_swap_supported(void (cleanup)(void), const char *filename)
{
	int fibmap = tst_fibmap(filename);
	long fs_type = tst_fs_type(cleanup, filename);
	const char *fstype = tst_fs_type_name(fs_type);

	int ret = make_swapfile(NULL, filename, 1);
	if (ret != 0) {
		if (fibmap == 1) {
			tst_brkm(TCONF, cleanup,
				"mkswap on %s not supported", fstype);
		} else {
			tst_brkm(TFAIL, cleanup,
				"mkswap on %s failed", fstype);
		}
	}

	TEST(ltp_syscall(__NR_swapon, filename, 0));
	if (TEST_RETURN == -1) {
		if (fibmap == 1 && errno == EINVAL) {
			tst_brkm(TCONF, cleanup,
				"Swapfile on %s not implemented", fstype);
		} else {
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "swapon on %s failed", fstype);
		}
	}

	TEST(ltp_syscall(__NR_swapoff, filename, 0));
	if (TEST_RETURN == -1) {
		tst_brkm(TFAIL | TERRNO, cleanup,
			"swapoff on %s failed", fstype);
	}
}
