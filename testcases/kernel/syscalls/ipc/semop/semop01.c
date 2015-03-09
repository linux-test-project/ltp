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
 *	semop01.c
 *
 * DESCRIPTION
 *	semop01 - test that semop() basic functionality is correct
 *
 * ALGORITHM
 *	create a semaphore set and initialize some values
 *	loop if that option was specified
 *	call semop() to set values for the primitive semaphores
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing
 *		get the semaphore values and compare with expected values
 *		if correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	  else issue a PASS message
 *	call cleanup
 *
 * HISTORY
 *	03/2001  - Written by Wayne Boyer
 *	17/01/02 - Modified. Manoj Iyer, IBM Austin. TX. manjo@austin.ibm.com
 *	           4th argument to semctl() system call was modified according
 *	           to man pages.
 *	           In my opinion The test should not even have compiled but
 *	           it was working due to some mysterious reason.
 *
 * RESTRICTIONS
 *	none
 */

#include "ipcsem.h"

#define NSEMS	4		/* the number of primitive semaphores to test */

char *TCID = "semop01";
int TST_TOTAL = 1;

int sem_id_1 = -1;		/* a semaphore set with read & alter permissions */

union semun get_arr;
struct sembuf sops[PSEMS];

int main(int ac, char **av)
{
	int lc;
	int i;
	int fail = 0;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(semop(sem_id_1, sops, NSEMS));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "%s call failed - errno = %d : %s",
				 TCID, TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			/* get the values and make sure they */
			/* are the same as what was set      */
			if (semctl(sem_id_1, 0, GETALL, get_arr) == -1) {
				tst_brkm(TBROK, cleanup,
					 "semctl() failed in functional test");
			}

			for (i = 0; i < NSEMS; i++) {
				if (get_arr.array[i] != i * i) {
					fail = 1;
				}
			}
			if (fail)
				tst_resm(TFAIL,
					 "semaphore values are wrong");
			else
				tst_resm(TPASS,
					 "semaphore values are correct");
		}

		/*
		 * clean up things in case we are looping
		 */
		union semun set_arr;
		set_arr.val = 0;
		for (i = 0; i < NSEMS; i++) {
			if (semctl(sem_id_1, i, SETVAL, set_arr) == -1) {
				tst_brkm(TBROK, cleanup, "semctl failed");
			}
		}
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	int i;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	get_arr.array = malloc(sizeof(unsigned short int) * PSEMS);
	if (get_arr.array == NULL)
		tst_brkm(TBROK, cleanup, "malloc failed");

	semkey = getipckey();

	sem_id_1 = semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);
	if (sem_id_1 == -1)
		tst_brkm(TBROK, cleanup, "couldn't create semaphore in setup");

	for (i = 0; i < NSEMS; i++) {
		sops[i].sem_num = i;
		sops[i].sem_op = i * i;
		sops[i].sem_flg = SEM_UNDO;
	}
}

void cleanup(void)
{
	rm_sema(sem_id_1);

	free(get_arr.array);

	tst_rmdir();
}
