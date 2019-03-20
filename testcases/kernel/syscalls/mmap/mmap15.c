/*
 * Copyright (c) International Business Machines  Corp., 2004
 *  Written by Robbie Williamson
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
 * Test Description: Test that a normal page cannot be mapped into a high
 * memory region.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/abisize.h"

char *TCID = "mmap15";
int TST_TOTAL = 1;

#ifdef __ia64__
# define HIGH_ADDR (void *)(0xa000000000000000UL)
#else
# define HIGH_ADDR (void *)(-page_size)
#endif

static long page_size;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc, fd;
	void *addr;

#ifdef TST_ABI32
	tst_brkm(TCONF, NULL, "This test is only for 64bit");
#endif

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		fd = SAFE_OPEN(cleanup, "testfile", O_RDWR | O_CREAT, 0666);

		/* Attempt to mmap into highmem addr, should get ENOMEM */
		addr = mmap(HIGH_ADDR, page_size, PROT_READ,
			    MAP_SHARED | MAP_FIXED, fd, 0);
		if (addr != MAP_FAILED) {
			tst_resm(TFAIL, "mmap into high region "
				 "succeeded unexpectedly");
			munmap(addr, page_size);
			close(fd);
			continue;
		}

		if (errno != ENOMEM && errno != EINVAL) {
			tst_resm(TFAIL | TERRNO, "mmap into high region "
				 "failed unexpectedly");
		} else {
			tst_resm(TPASS | TERRNO, "mmap into high region "
				 "failed as expected");
		}

		SAFE_CLOSE(cleanup, fd);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	tst_tmpdir();

	page_size = getpagesize();

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}
