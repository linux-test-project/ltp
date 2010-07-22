/*
 * Copyright (c) International Business Machines  Corp., 2005
 * Copyright (c) Wipro Technologies Ltd, 2005.  All Rights Reserved.
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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : pselect01
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : Basic tests for pselect01(2)
 *
 *    TEST CASE TOTAL   : 1
 *
 *    AUTHOR            : Prashant P Yendigeri
 *                        <prashant.yendigeri@wipro.com>
 *                        Robbie Williamson
 *                        <robbiew@us.ibm.com>
 *
 *    DESCRIPTION
 *      This is a Phase I test for the pselect01(2) system call.
 *      It is intended to provide a limited exposure of the system call.
 *
 **********************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "pselect01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

#define FILENAME "pselect01_test"
#define LOOP_COUNT 4

int main()
{
	int ret_pselect, fd;
	fd_set readfds;
	int retval;
	struct timespec tv, tv_start, tv_end;
	long real_nsec, total_nsec;
	double real_sec;
	int total_sec;

	setup();

	fd = open(FILENAME, O_CREAT | O_RDWR, 0777);
	if (fd < 0) {
		tst_resm(TBROK, "Opening %s...Failed....err %d", FILENAME,
			 errno);
		cleanup();
	}
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	tv.tv_sec = 0;
	tv.tv_nsec = 0;

	ret_pselect = pselect(fd, &readfds, 0, 0, &tv, NULL);
	if (ret_pselect >= 0) {
		tst_resm(TPASS, "Basic pselect syscall testing....OK");
	} else
		tst_resm(TFAIL,
			 "Basic pselect syscall testing....FAILED, err %d",
			 errno);
	close(fd);
	remove(FILENAME);

	for (total_sec = 1; total_sec <= LOOP_COUNT; total_sec++) {
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);

		tv.tv_sec = total_sec;
		tv.tv_nsec = 0;

		tst_resm(TINFO,
			 "Testing basic pselect sanity,Sleeping for %jd secs",
			 (intmax_t)tv.tv_sec);
		clock_gettime(CLOCK_REALTIME, &tv_start);
		retval =
		    pselect(0, &readfds, NULL, NULL, &tv,
			    NULL);
		clock_gettime(CLOCK_REALTIME, &tv_end);

		real_sec = (0.5 + (tv_end.tv_sec - tv_start.tv_sec + 
			    1e-9 * (tv_end.tv_nsec - tv_start.tv_nsec)));
		if (abs(real_sec - total_sec) < 0.2 * total_sec)
			tst_resm(TPASS, "Sleep time was correct "
				"(%lf/%d < 20 %%)", real_sec, total_sec);
		else
			tst_resm(TFAIL, "Sleep time was incorrect (%lf/%lf "
				">= 20%%)", total_sec, real_sec);
	}

#ifdef DEBUG
	tst_resm(TINFO, "Now checking nsec sleep precision");
#endif
	for (total_nsec = 1e8; total_nsec <= LOOP_COUNT * 1e8; total_nsec += 1e8) {
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);

		tv.tv_sec = 0;
		tv.tv_nsec = total_nsec;

		tst_resm(TINFO,
			 "Testing basic pselect sanity,Sleeping for %ld nano secs",
			 tv.tv_nsec);
		clock_gettime(CLOCK_REALTIME, &tv_start);
		retval =
		    pselect(0, &readfds, NULL, NULL, &tv,
			    NULL);
		clock_gettime(CLOCK_REALTIME, &tv_end);

		/* Changed total_sec compare to an at least vs an exact compare */

		real_nsec = (tv_end.tv_sec - tv_start.tv_sec) * 1e9 +
			tv_end.tv_nsec - tv_start.tv_nsec;

		/* allow 20% error*/
		if (abs(real_nsec - tv.tv_nsec) < 0.2 * total_nsec)
			tst_resm(TPASS, "Sleep time was correct");
		else {
			tst_resm(TWARN,
				 "This test could fail if the system was under load");
			tst_resm(TWARN,
				 "due to the limitation of the way it calculates the");
			tst_resm(TWARN, "system call execution time.");
			tst_resm(TFAIL,
				"Sleep time was incorrect:%ld nsec vs expected %ld nsec",
				 real_nsec, total_nsec);
		}
	}
	cleanup();
	return 0;
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* create temporary directory */
	tst_tmpdir();

	/* Pause if that option was specified */
	TEST_PAUSE;
}				/* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* remove temporary test directory */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
