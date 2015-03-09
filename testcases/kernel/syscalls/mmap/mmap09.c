/*
 * Copyright (c) International Business Machines  Corp., 2003
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
 * Test Description:
 *  Verify that truncating a mmaped file works correctly.
 *
 * Expected Result:
 *  ftruncate should be allowed to increase, decrease, or zero the
 *  size of a file that has been mmaped
 *
 *  Test:
 *   Use ftruncate to shrink the file while it is mapped
 *   Use ftruncate to grow the file while it is mapped
 *   Use ftruncate to zero the size of the file while it is mapped
 *
 * HISTORY
 *	04/2003 Written by Paul Larson
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "test.h"

#define mapsize (1 << 14)

char *TCID = "mmap09";
int TST_TOTAL = 3;

static int fd;
static char *maddr;

static struct test_case_t {
	off_t newsize;
	char *desc;
} TC[] = {
	{mapsize - 8192, "ftruncate mmaped file to a smaller size"},
	{mapsize + 1024, "ftruncate mmaped file to a larger size"},
	{0, "ftruncate mmaped file to 0 size"},
};

static void setup(void);
static void cleanup(void);

int main(int argc, char **argv)
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(ftruncate(fd, TC[i].newsize));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL | TTERRNO, "%s", TC[i].desc);
			} else {
				tst_resm(TPASS, "%s", TC[i].desc);
			}
		}

	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if ((fd = open("mmaptest", O_RDWR | O_CREAT, 0666)) < 0)
		tst_brkm(TFAIL | TERRNO, cleanup, "opening mmaptest failed");

	/* ftruncate the file to 16k */
	if (ftruncate(fd, mapsize) < 0)
		tst_brkm(TFAIL | TERRNO, cleanup, "ftruncate file failed");

	maddr = mmap(0, mapsize, PROT_READ | PROT_WRITE,
		     MAP_FILE | MAP_SHARED, fd, 0);
	if (maddr == MAP_FAILED)
		tst_brkm(TFAIL | TERRNO, cleanup, "mmapping mmaptest failed");

	/* fill up the file with A's */
	memset(maddr, 'A', mapsize);
}

static void cleanup(void)
{
	munmap(maddr, mapsize);
	close(fd);
	tst_rmdir();
}
