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
 *	umask03.c
 *
 * DESCRIPTION
 *	Check that umask changes the mask, and that the previous
 *	value of the mask is returned correctly for each value.
 *
 * ALGORITHM
 *	For each mask value (9 bits) set mask, and check that the return
 *	corresponds to the previous value set.
 *
 * USAGE:  <for command-line>
 *		umask03 [-c n] [-i n] [-I x] [-P x] [-t]
 *		where,  -c n : Run n copies concurrently.
 *			-i n : Execute test n times.
 *			-I x : Execute test for x seconds.
 *			-P x : Pause for x seconds between iterations.
 *			-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *	None
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "test.h"
#include "usctest.h"
#include <sys/types.h>
#include <sys/stat.h>

char *TCID = "umask03";
int TST_TOTAL = 1;
extern int Tst_count;

char filname[40];

void setup(void);
void cleanup(void);

int main(int argc, char **argv)
{
	int lc;
	char *msg;

	struct stat statbuf;
	int mskval = 0000;
	int failcnt = 0;
	int fildes, i;
	unsigned low9mode;

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL))
	    != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	setup();		/* global setup */

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (umask(mskval = 0077), i = 1; mskval < 01000;
		     i++, umask(++mskval)) {
			unlink(filname);
			if ((fildes = creat(filname, 0777)) == -1) {
				tst_resm(TBROK, "cannot create "
					 "file with mskval 0%o %d",
					 mskval, mskval);
			} else {
				if (fstat(fildes, &statbuf) != 0) {
					tst_resm(TBROK, "cannot fstat file");
				} else {
					low9mode = statbuf.st_mode & 0777;
					if (low9mode != (~mskval & 0777)) {
						tst_resm(TFAIL,
							 "got %o expected %o"
							 "mask didnot take",
							 low9mode,
							 (~mskval & 0777));
					 /*NOTREACHED*/}
				}
			}
			close(fildes);
		}
		if (!failcnt)
			tst_resm(TPASS, "umask correctly returns the "
				 "previous value for all masks");
	}
	cleanup();
	 /*NOTREACHED*/ return 0;

}

/*
 * setup
 *	performs all ONE TIME setup for this test
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* make temp dir and cd to it */
	tst_tmpdir();

	sprintf(filname, "umask2.%d", getpid());
}

/*
 * cleanup
 *	performs all ONE TIME cleanup for this test at completion or
 *	premature exit
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/*
	 * cleanup the temporary files and the temporary directory
	 */
	unlink(filname);
	tst_rmdir();

	/*
	 * exit with return code appropriate for results
	 */
	tst_exit();
}
