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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * AUTHORS:
 *    Prashant P Yendigeri <prashant.yendigeri@wipro.com>
 *    Robbie Williamson <robbiew@us.ibm.com>
 *
 * DESCRIPTION
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
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

TCID_DEFINE(pselect01);
int TST_TOTAL = 9;

#define FILENAME "pselect01_test"
#define LOOP_COUNT 4

static int fd;

static void pselect_verify(void)
{
	fd_set readfds;
	struct timespec tv, tv_start, tv_end;
	long real_nsec, total_nsec;
	double real_sec;
	int total_sec, retval;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	tv.tv_sec = 0;
	tv.tv_nsec = 0;

	retval = pselect(fd, &readfds, 0, 0, &tv, NULL);
	if (retval >= 0)
		tst_resm(TPASS, "pselect() succeeded retval=%i", retval);
	else
		tst_resm(TFAIL | TERRNO, "pselect() failed unexpectedly");

	for (total_sec = 1; total_sec <= LOOP_COUNT; total_sec++) {
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);

		tv.tv_sec = total_sec;
		tv.tv_nsec = 0;

		tst_resm(TINFO,
			 "Testing basic pselect sanity,Sleeping for %jd secs",
			 (intmax_t) tv.tv_sec);
		clock_gettime(CLOCK_MONOTONIC, &tv_start);
		pselect(0, &readfds, NULL, NULL, &tv, NULL);
		clock_gettime(CLOCK_MONOTONIC, &tv_end);

		real_sec = (0.5 + (tv_end.tv_sec - tv_start.tv_sec +
				   1e-9 * (tv_end.tv_nsec - tv_start.tv_nsec)));
		if (abs(real_sec - total_sec) < 0.2 * total_sec)
			tst_resm(TPASS, "Sleep time was correct "
				 "(%lf/%d < 20 %%)", real_sec, total_sec);
		else
			tst_resm(TFAIL, "Sleep time was incorrect (%d/%lf "
				 ">= 20%%)", total_sec, real_sec);
	}

#ifdef DEBUG
	tst_resm(TINFO, "Now checking nsec sleep precision");
#endif
	for (total_nsec = 1e8; total_nsec <= LOOP_COUNT * 1e8;
	     total_nsec += 1e8) {
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);

		tv.tv_sec = 0;
		tv.tv_nsec = total_nsec;

		tst_resm(TINFO,
			 "Testing basic pselect sanity,Sleeping for %ld nano secs",
			 tv.tv_nsec);
		clock_gettime(CLOCK_MONOTONIC, &tv_start);
		pselect(0, &readfds, NULL, NULL, &tv, NULL);
		clock_gettime(CLOCK_MONOTONIC, &tv_end);

		real_nsec = (tv_end.tv_sec - tv_start.tv_sec) * 1e9 +
		    tv_end.tv_nsec - tv_start.tv_nsec;

		/* allow 20% error */
		if (abs(real_nsec - tv.tv_nsec) < 0.2 * total_nsec) {
			tst_resm(TPASS, "Sleep time was correct");
		} else {
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
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		pselect_verify();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	tst_timer_check(CLOCK_MONOTONIC);
	tst_tmpdir();

	fd = SAFE_OPEN(cleanup, FILENAME, O_CREAT | O_RDWR, 0777);

	TEST_PAUSE;
}

static void cleanup(void)
{
	if (fd && close(fd))
		tst_resm(TWARN | TERRNO, "close() failed");

	tst_rmdir();
}
