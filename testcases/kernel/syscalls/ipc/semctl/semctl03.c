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
 *	semctl03.c
 *
 * DESCRIPTION
 *	semctl03 - test for EINVAL and EFAULT errors
 *
 * ALGORITHM
 *	create a semaphore set with read and alter permissions
 *	loop if that option was specified
 *	call semctl() using four different invalid cases
 *	check the errno value
 *	  issue a PASS message if we get EINVAL or EFAULT
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  semctl03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "ipcsem.h"

#ifdef _XLC_COMPILER
#define SEMUN_CAST
#else
#define SEMUN_CAST (union semun)
#endif

char *TCID = "semctl03";
int TST_TOTAL = 4;

#ifdef _XLC_COMPILER
#define SEMUN_CAST
#else
#define SEMUN_CAST (union semun)
#endif

int sem_id_1 = -1;
int bad_id = -1;

struct semid_ds sem_ds;

struct test_case_t {
	int *sem_id;
	int ipc_cmd;
	union semun arg;
	int error;
} TC[] = {
	/* EINVAL - the IPC command is not valid */
	{
	&sem_id_1, -1, SEMUN_CAST & sem_ds, EINVAL},
	    /* EINVAL - the semaphore ID is not valid */
	{
	&bad_id, IPC_STAT, SEMUN_CAST & sem_ds, EINVAL},
	    /* EFAULT - the union arg is invalid when expecting "ushort *array" */
	{
	&sem_id_1, GETALL, SEMUN_CAST - 1, EFAULT},
	    /* EFAULT - the union arg is invalid when expecting */
	    /* "struct semid_ds *buf */
	{
	&sem_id_1, IPC_SET, SEMUN_CAST - 1, EFAULT}
};

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(semctl(*(TC[i].sem_id), 0, TC[i].ipc_cmd,
				    TC[i].arg));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - errno = %d"
					 " : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			}
		}
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

	/* get an IPC resource key */
	semkey = getipckey();

	/* create a semaphore set with read and alter permissions */
	if ((sem_id_1 =
	     semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA)) == -1) {
		tst_brkm(TBROK, cleanup, "couldn't create semaphore in setup");
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/* if it exists, remove the semaphore resouce */
	rm_sema(sem_id_1);

	tst_rmdir();

}
