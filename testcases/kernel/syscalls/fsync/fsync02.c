/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	fsync02.c
 *
 * DESCRIPTION
 *	Create a sparse file, fsync it, and time the fsync
 *
 * ALGORITHM
 *	1. Create a file.
 *	2. Write to the file at equally spaced intervals up to a max block
 *	3. Check if the time limit was exceeded.
 *
 * USAGE:  <for command-line>
 *  fsync02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/resource.h>
#include "test.h"
#include "safe_macros.h"
#include <time.h>

#define BLOCKSIZE 8192
#define MAXBLKS 262144
#define TIME_LIMIT 120

char *TCID = "fsync02";
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);

char tempfile[40] = "";
int fd, pid;
off_t max_blks = MAXBLKS;

struct statvfs stat_buf;

int main(int ac, char **av)
{
	int lc;

	off_t offsetret, offset;
	char pbuf[BUFSIZ];
	int ret, max_block = 0;
	int i;
	time_t time_start, time_end;
	double time_delta;
	int data_blocks = 0;
	long int random_number;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		while (max_block <= data_blocks) {
			random_number = random();
			max_block = random_number % max_blks;
			data_blocks = random_number % 1000 + 1;
		}

		for (i = 1; i <= data_blocks; i++) {
			offset = i * ((BLOCKSIZE * max_block) / data_blocks);
			offset -= BUFSIZ;
			if ((offsetret = lseek(fd, offset, SEEK_SET)) != offset)
				tst_brkm(TBROK | TERRNO, cleanup,
					 "lseek failed: %ld, %ld", offsetret,
					 offset);
			if ((ret = write(fd, pbuf, BUFSIZ)) != BUFSIZ)
				tst_brkm(TBROK, cleanup, "write failed");
		}
		if (time(&time_start) == -1)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "getting start time failed");

		TEST(fsync(fd));

		if (time(&time_end) == -1)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "getting end time failed");

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "fsync failed");
			continue;
		}

		if (time_end < time_start)
			tst_resm(TBROK,
				 "timer broken end %ld < start %ld",
				 time_end, time_start);

		if ((time_delta =
		     difftime(time_end, time_start)) > TIME_LIMIT)
			tst_resm(TFAIL,
				 "fsync took too long: %lf seconds; "
				 "max_block: %d; data_blocks: %d",
				 time_delta, max_block, data_blocks);
		else
			tst_resm(TPASS, "fsync succeeded in an "
				 "acceptable amount of time");

		SAFE_FTRUNCATE(cleanup, fd, 0);
	}

	sync();
	cleanup();
	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	/* free blocks avail to non-superuser */
	unsigned long f_bavail;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	sprintf(tempfile, "%s.%d", TCID, pid = getpid());
	srand48(pid);

	if ((fd = open(tempfile, O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1)
		tst_brkm(TBROK, cleanup, "open failed");

	if (fstatvfs(fd, &stat_buf) != 0)
		tst_brkm(TBROK, cleanup, "fstatvfs failed");

	f_bavail = (stat_buf.f_bavail * stat_buf.f_frsize) / BLOCKSIZE;
	if (f_bavail && (f_bavail < MAXBLKS))
		max_blks = f_bavail;

#ifdef LARGEFILE
	if ((fcntl(fd, F_SETFL, O_LARGEFILE)) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "fcntl(.., O_LARGEFILE) failed");

	if (write(fd, pbuf, BUFSIZ) != BUFSIZ)
		tst_brkm(TBROK | TERRNO, cleanup, "write(fd, pbuf, ..) failed");
#endif
}

void cleanup(void)
{
	if (close(fd) == -1)
		tst_resm(TWARN | TERRNO, "close failed");

	tst_rmdir();

}
