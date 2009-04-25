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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
 *	3. Check if time limit exceeded.
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
#include <test.h>
#include <usctest.h>
#include <time.h>

#define BLOCKSIZE 8192
#define MAXBLKS 262144
#define TIME_LIMIT 120

char *TCID = "fsync02";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

char tempfile[40] = "";
int fd, pid;
off_t max_blks = MAXBLKS;

struct statvfs stat_buf;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	off_t offsetret, offset;
	char pbuf[BUFSIZ];
	int ret, max_block = 0;
	int i;
	time_t time_start, time_end;
	double time_delta;
	int data_blocks = 0;
	long int random_number;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		while (max_block <= data_blocks) {
			random_number = random();
			max_block = random_number % max_blks;
			data_blocks = random_number % 1000 + 1;
		}

		for (i = 0; i < data_blocks; i++) {
			if ((offsetret = lseek(fd, offset = BLOCKSIZE
					       * max_block / data_blocks * (i +
									    1)
					       - BUFSIZ, SEEK_SET)) != offset) {
				tst_brkm(TBROK, cleanup, "lseek failed");
			}
			if ((ret = write(fd, pbuf, BUFSIZ)) != BUFSIZ) {
				tst_brkm(TBROK, cleanup, "Cannot write "
					 "to file");
			}
		}
		time(&time_start);

		TEST(fsync(fd));

		time(&time_end);

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "fsync failed - %d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		if (STD_FUNCTIONAL_TEST) {
			if (time_end < time_start) {
				tst_resm(TFAIL,
					 "timer broken end %ld < start %ld",
					 time_end, time_start);
			}

			if ((time_delta =
			     difftime(time_end, time_start)) > TIME_LIMIT) {
				tst_resm(TFAIL,
					 "fsync took too long: %d "
					 "seconds; max_block: %d; data_blocks: "
					 "%d", (int)time_delta, max_block,
					 data_blocks);
			} else {
				tst_resm(TPASS, "fsync() succeeded in an "
					 "acceptable amount of time");
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}

		if (ftruncate(fd, 0) < 0) {
			tst_brkm(TBROK, cleanup, "ftruncate failed");
		}
	}
	close(fd);
	sync();
	cleanup();
	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* free blocks avail to non-superuser */
	unsigned long f_bavail;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	sprintf(tempfile, "%s.%d", TCID, pid = getpid());
	srand48(pid);

	if ((fd = open(tempfile, O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1) {
		tst_brkm(TBROK, cleanup, "Can't open temp file");
	}

	if (fstatvfs(fd, &stat_buf) != 0) {
		tst_brkm(TBROK, cleanup, "Can't get the information about the "
			 "file system");
	}

	f_bavail = stat_buf.f_bavail / (BLOCKSIZE / stat_buf.f_frsize);
	if (f_bavail && (f_bavail < MAXBLKS))
		max_blks = f_bavail;

#ifdef LARGEFILE
	if ((fcntl(fd, F_SETFL, O_LARGEFILE)) == -1) {
		tst_brkm(TBROK, cleanup, "fcntl failed to O_LARGEFILE");
	}

	if (write(fd, pbuf, BUFSIZ) != BUFSIZ) {
		tst_brkm(TBROK, cleanup, "Cannot write to tempfile");
	}
#endif
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
