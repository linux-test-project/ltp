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
 *	shmctl04.c
 *
 * DESCRIPTION
 *	shmctl04 - test the SHM_INFO command
 *		   they are used with shmctl() in ipcs
 *
 * USAGE:  <for command-line>
 *  shmctl04 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	09/2002 - Written by Mingming Cao
 *
 * RESTRICTIONS
 *	none
 */

#include "ipcshm.h"

char *TCID = "shmctl04";
int TST_TOTAL = 1;

struct shm_info shm_info;
int max_ids;

/*
 * These are the various setup and check functions for the commands
 * that we are checking.
 */

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;
		TEST(shmctl(0, SHM_INFO, (struct shmid_ds *)&shm_info));

		if (TEST_RETURN != -1) {
			tst_resm(TPASS, "SHM_INFO call succeeded");
			continue;
		}

		tst_resm(TFAIL, "SHM_INFO call failed with an unexpected error"
			 " - %d : %s", TEST_ERRNO, strerror(TEST_ERRNO));

	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{

	tst_rmdir();

}
