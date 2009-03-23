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
 *	fcntl13.c
 *
 * DESCRIPTION
 *	Testcase to test that fcntl() sets errno correctly.
 *
 * USAGE
 *	fcntl13
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	NONE
 */

#include <fcntl.h>
#include <errno.h>
#include <test.h>
#include <usctest.h>

#define F_BADCMD 99999

char *TCID = "fcntl13";
int TST_TOTAL = 1;
extern int Tst_count;

int fail;
void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	struct flock flock;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

/* //block1: */
		tst_resm(TINFO, "Enter block 1");
		tst_resm(TINFO, "Test for errno EINVAL");
		fail = 0;

		if (fcntl(1, F_BADCMD, 1) != -1) {
			tst_resm(TFAIL, "fcntl(2) failed to FAIL");
			fail = 1;
		} else if (errno != EINVAL) {
			tst_resm(TFAIL, "Expected EINVAL got %d", errno);
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "block 1 FAILED");
		} else {
			tst_resm(TINFO, "block 1 PASSED");
		}
		tst_resm(TINFO, "Exit block 1");

/* //block2: */
#ifndef UCLINUX
		/* Skip since uClinux does not implement memory protection */
		tst_resm(TINFO, "Enter block 2");
		tst_resm(TINFO, "Test for errno EFAULT");
		fail = 0;

		/* case 1: F_SETLK */
		if (fcntl(1, F_SETLK, (void *)-1) != -1) {
			tst_resm(TFAIL, "F_SETLK: fcntl(2) failed to FAIL");
			fail = 1;
		} else if (errno != EFAULT) {
			tst_resm(TFAIL, "F_SETLK: Expected EFAULT got %d",
				 errno);
			fail = 1;
		}

		/* case 2: F_SETLKW */
		if (fcntl(1, F_SETLKW, (void *)-1) != -1) {
			tst_resm(TFAIL, "F_SETLKW: fcntl(2) failed to FAIL");
			fail = 1;
		} else if (errno != EFAULT) {
			tst_resm(TFAIL, "F_SETLKW: Expected EFAULT got %d",
				 errno);
			fail = 1;
		}

		/* case 3: F_GETLK */
		if (fcntl(1, F_GETLK, (void *)-1) != -1) {
			tst_resm(TFAIL, "F_GETLK: fcntl(2) failed to FAIL");
			fail = 1;
		} else if (errno != EFAULT) {
			tst_resm(TFAIL, "F_GETLK: Expected EFAULT got %d",
				 errno);
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "blcok 2 FAILED");
		} else {
			tst_resm(TINFO, "block 2 PASSED");
		}
		tst_resm(TINFO, "Exit block 2");
#else
		tst_resm(TINFO, "Skip block 2 on uClinux");
#endif

/* //block3: */
		tst_resm(TINFO, "Enter block 3");
		tst_resm(TINFO, "Test for errno EINVAL");
		fail = 0;

		flock.l_whence = -1;
		flock.l_type = F_WRLCK;
		flock.l_start = 0L;
		flock.l_len = 0L;

		if (fcntl(1, F_SETLK, &flock) != -1) {
			tst_resm(TFAIL, "fcntl(2) failed to FAIL");
			fail = 1;
		} else if (errno != EINVAL) {
			tst_resm(TFAIL, "Expected EINVAL, got %d", errno);
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "block 3 FAILED");
		} else {
			tst_resm(TINFO, "block 3 PASSED");
		}
		tst_resm(TINFO, "Exit block 3");

/* //block4: */
		tst_resm(TINFO, "Enter block 4");
		tst_resm(TINFO, "Test for errno EBADF");
		fail = 0;

		if (fcntl(-1, F_GETLK, &flock) != -1) {
			tst_resm(TFAIL, "fcntl(2) failed to FAIL");
			fail = 1;
		} else if (errno != EBADF) {
			tst_resm(TFAIL, "Expected EBADF, got %d", errno);
			fail = 1;
		}

		if (fail) {
			tst_resm(TINFO, "block 4 FAILED");
		} else {
			tst_resm(TINFO, "block 4 PASSED");
		}
		tst_resm(TINFO, "Exit block 4");
	}
	cleanup();
	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
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

	/* exit with return code appropriate for results */
	tst_exit();
}
