/*
 *
 *   Copyright (c) International Business Machines  Corp., 2004
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
 *	hugeshmctl02.c
 *
 * DESCRIPTION
 *	hugeshmctl02 - check for EACCES, EFAULT and EINVAL errors
 *
 * ALGORITHM
 *	create a large shared memory segment without read or write permissions
 *	create a large shared memory segment with read & write permissions
 *	loop if that option was specified
 *	  call shmctl() using five different invalid cases
 *	  check the errno value
 *	    issue a PASS message if we get EACCES, EFAULT or EINVAL
 *	  otherwise, the tests fails
 *	    issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  hugeshmctl02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 *
 * RESTRICTIONS
 *	none
 */

#include "ipcshm.h"
#include <pwd.h>
#include "system_specific_hugepages_info.h"

char *TCID = "hugeshmctl02";
int TST_TOTAL = 4;
extern int Tst_count;
char nobody_uid[] = "nobody";
struct passwd *ltpuser;
unsigned long huge_pages_shm_to_be_allocated;

int exp_enos[] = {EPERM, EACCES, EFAULT, EINVAL, 0};  /* 0 terminated list  */
						      /* of expected errnos */
int shm_id_1 = -1;
int shm_id_2 = -1;
int shm_id_3 = -1;

struct shmid_ds buf;

struct test_case_t {
	int *shmid;
	int cmd;
	struct shmid_ds *sbuf;
	int error;
} TC[] = {
	/* EFAULT - IPC_SET & buf isn't valid */
	{&shm_id_2, IPC_SET, (struct shmid_ds *)-1, EFAULT},

	/* EFAULT - IPC_STAT & buf isn't valid */
	{&shm_id_2, IPC_STAT, (struct shmid_ds *)-1, EFAULT},

	/* EINVAL - the shmid is not valid */
	{&shm_id_3, IPC_STAT, &buf, EINVAL},

	/* EINVAL - the command is not valid */
	{&shm_id_2, -1, &buf, EINVAL},
};

int main(int ac, char **av)
{
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */
	int i;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

        if ( get_no_of_hugepages() <= 0 || hugepages_size() <= 0 )
             tst_brkm(TCONF, tst_exit, "Not enough available Hugepages");
        else             
             huge_pages_shm_to_be_allocated = ( get_no_of_hugepages() * hugepages_size() * 1024) / 2 ;

	setup();			/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* loop through the test cases */
		for (i=0; i<TST_TOTAL; i++) {
			/*
			 * use the TEST() macro to make the call
			 */

			TEST(shmctl(*(TC[i].shmid), TC[i].cmd, TC[i].sbuf));

			if ((TEST_RETURN != -1)&&(i < 5)) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - errno = "
					 "%d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "call failed with an "
					 "unexpected error - %d : %s",
				 	TEST_ERRNO, strerror(TEST_ERRNO));
			}		
		}
	}

	cleanup();

	/*NOTREACHED*/
	return 0;
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void
setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	/* get an IPC resource key */
	shmkey = getipckey();

	/* create a shared memory segment without read or write permissions */
	if ((shm_id_1 = shmget(shmkey, huge_pages_shm_to_be_allocated, SHM_HUGETLB | IPC_CREAT | IPC_EXCL)) == -1) {
		tst_brkm(TBROK, cleanup, "couldn't create shared memory "
			 "segment #1 in setup()");
	}

	/* create a shared memory segment with read and write permissions */
	if ((shm_id_2 = shmget(shmkey + 1, huge_pages_shm_to_be_allocated, SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW)) == -1) {
		tst_brkm(TBROK, cleanup, "couldn't create shared memory "
			 "segment #2 in setup()");
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void
cleanup(void)
{
	/* if they exist, remove the shared memory resources */
	rm_shm(shm_id_1);
	rm_shm(shm_id_2);

	/* Remove the temporary directory */
	tst_rmdir();

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}

