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
 *	semget05.c
 *
 * DESCRIPTION
 *	semget05 - test for ENOSPC error
 *
 * ALGORITHM
 *	create semaphore sets in a loop until the system limit is reached
 *	loop if that option was specified
 *	attempt to create yet another semaphore set
 *	check the errno value
 *	  issue a PASS message if we get ENOSPC
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *      07/2006 - Changes By Michael Reed
 *                - Changed the value of MAXIDS for the specific machine by reading
 *                  the system limit for SEMMNI - The maximum number of sempahore sets
 *      03/2008 - Matthieu Fertré  (mfertre@irisa.fr)
 *                - Fix concurrency issue. Create private semaphores to
 *                  avoid conflict with concurrent processes.
 *
 * RESTRICTIONS
 *	none
 */

#include "../lib/ipcsem.h"

char *TCID = "semget05";
int TST_TOTAL = 1;

/*
 * The MAXIDS value is somewhat arbitrary and may need to be increased
 * depending on the system being tested.
 */

int MAXIDS = 2048;

int *sem_id_arr = NULL;
int num_sems = 0;		/* count the semaphores created */

int main(int ac, char **av)
{
	int lc;
	FILE *fp;

	tst_parse_opts(ac, av, NULL, NULL);

	/* Set the MAXIDS for the specific machine by reading the system limit
	 * for SEMMNI - The maximum number of sempahore sets
	 */
	fp = fopen("/proc/sys/kernel/sem", "r");
	if (fp != NULL) {
		int getmaxid;
		if (fscanf(fp, "%*d %*d %*d %d", &getmaxid) == 1)
			MAXIDS = getmaxid + 1;
		fclose(fp);
	}

	sem_id_arr = malloc(sizeof(int) * MAXIDS);
	if (sem_id_arr == NULL)
		tst_brkm(TBROK, cleanup, "malloc failed");

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;


		TEST(semget(IPC_PRIVATE, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA));
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded when error expected");
			continue;
		}

		switch (TEST_ERRNO) {
		case ENOSPC:
			tst_resm(TPASS, "expected failure - errno "
				 "= %d : %s", TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "unexpected error - %d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		}
	}

	cleanup();

	tst_exit();
}

void setup(void)
{
	int sem_q;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	while ((sem_q = semget(IPC_PRIVATE, PSEMS, IPC_CREAT | IPC_EXCL)) != -1) {
		sem_id_arr[num_sems++] = sem_q;
		if (num_sems == MAXIDS) {
			tst_brkm(TBROK, cleanup, "The maximum number of "
				 "semaphore ID's has been\n\t reached.  Please "
				 "increase the MAXIDS value in the test.");
		}
	}

	if (errno != ENOSPC) {
		tst_brkm(TBROK, cleanup, "Didn't get ENOSPC in test setup"
			 " - errno = %d : %s", errno, strerror(errno));
	}
}

void cleanup(void)
{
	int i;

	for (i = 0; i < num_sems; i++) {
		rm_sema(sem_id_arr[i]);
	}

	free(sem_id_arr);
	tst_rmdir();
}
