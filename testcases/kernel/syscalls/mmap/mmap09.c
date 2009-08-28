/*
 *
 *   Copyright (c) International Business Machines  Corp., 2003
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Test Name: mmap09
 *
 * Test Description:
 *  Verify that truncating a mmaped file works correctly.
 *
 * Expected Result:
 *  ftruncate should be allowed to increase, decrease, or zero the
 *  size of a file that has been mmaped
 *
 * Algorithm:
 *  Setup:
 *   Create file
 *   mmap the file
 *   fill it with data
 *
 *  Test:
 *   Use ftruncate to shrink the file while it is mapped
 *   Use ftruncate to grow the file while it is mapped
 *   Use ftruncate to zero the size of the file while it is mapped
 *
 * Usage:  <for command-line>
 *  mmap09 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	04/2003 Written by Paul Larson
 *
 * RESTRICTIONS:
 *  None.
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

#define mapsize (1 << 14)

char *TCID = "mmap09";
int TST_TOTAL = 3;
extern int Tst_count;
int fd;
char *maddr;

struct test_case_t {
	off_t newsize;
	char *desc;
} TC[] = {
	{
	mapsize - 8192, "ftruncate mmaped file to a smaller size"}, {
	mapsize + 1024, "ftruncate mmaped file to a larger size"}, {
0, "ftruncate mmaped file to 0 size"},};

int main(int argc, char **argv)
{
	int lc;
	int i;
	char *msg;		/* for parse_opts */

	msg = parse_opts(argc, argv, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(ftruncate(fd, TC[i].newsize));

			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL|TTERRNO, "%s", TC[i].desc);
			} else {
				tst_resm(TPASS, "%s", TC[i].desc);
			}
		}

	}

	cleanup();

	return 0;
}

void setup()
{
	int i;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if option was specified */
	TEST_PAUSE;

	tst_tmpdir();

	if ((fd = open("mmaptest", O_RDWR | O_CREAT, 0666)) < 0)
		tst_brkm(TFAIL|TERRNO, cleanup, "open(mmaptest) file failed");

	/* ftruncate the file to 16k */
	if (ftruncate(fd, mapsize) < 0)
		tst_brkm(TFAIL|TERRNO, cleanup, "ftruncate() file failed");

	maddr = mmap(0, (size_t) mapsize, PROT_READ | PROT_WRITE,
		     MAP_FILE | MAP_SHARED, fd, (off_t) 0);
	if (maddr == MAP_FAILED)
		tst_brkm(TFAIL|TERRNO, cleanup, "mmap() file failed");
	/* fill up the file with A's */
	for (i = 0; i < mapsize; i++)
		maddr[i] = 'A';

}

void cleanup()
{
	TEST_CLEANUP;
	munmap(maddr, (size_t) mapsize);
	close(fd);
	tst_rmdir();
	tst_exit();
}
