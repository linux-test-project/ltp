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

#include "test.h"
#include "libswapon.h"

/*
 * Make a swap file
 */
void make_swapfile(void (cleanup)(void), const char *swapfile)
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

	tst_run_cmd(cleanup, argv, "/dev/null", "/dev/null", 0);
}
