/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
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
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Testcase to check that creat(2) system call returns EMFILE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <unistd.h>
#include "tst_test.h"

static int *opened_fds, num_opened_fds;

static void verify_creat(void)
{
	TEST(creat("filename", 0666));

	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		SAFE_CLOSE(TEST_RETURN);
		return;
	}

	if (TEST_ERRNO == EMFILE)
		tst_res(TPASS, "creat() failed with EMFILE");
	else
		tst_res(TFAIL | TTERRNO, "Expected EMFILE");
}

static void setup(void)
{
	int max_open;
	int fd;
	char fname[PATH_MAX];

	/* get the maximum number of files that we can open */
	max_open = getdtablesize();
	tst_res(TINFO, "getdtablesize() = %d", max_open);
	opened_fds = SAFE_MALLOC(max_open * sizeof(int));

	/* now open as many files as we can up to max_open */
	do {
		snprintf(fname, sizeof(fname), "creat05_%d", num_opened_fds);
		fd = SAFE_CREAT(fname, 0666);
		opened_fds[num_opened_fds++] = fd;
	} while (fd < max_open - 1);

	tst_res(TINFO, "Opened additional #%d fds", num_opened_fds);
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < num_opened_fds; i++) {
		if (close(opened_fds[i])) {
			tst_res(TWARN | TERRNO, "close(%i) failed",
				opened_fds[i]);
		}
	}

	free(opened_fds);
}

static struct tst_test test = {
	.test_all = verify_creat,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
};
