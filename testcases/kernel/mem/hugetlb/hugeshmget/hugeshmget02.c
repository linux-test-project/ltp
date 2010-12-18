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
 *	hugeshmget02.c
 *
 * DESCRIPTION
 *	hugeshmget02 - check for ENOENT, EEXIST and EINVAL errors
 *
 * ALGORITHM
 *	create a large shared memory segment with read & write permissions
 *	loop if that option was specified
 *	  call shmget() using five different invalid cases
 *	  check the errno value
 *	    issue a PASS message if we get ENOENT, EEXIST or EINVAL
 *	  otherwise, the tests fails
 *	    issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  hugeshmget02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
#include "system_specific_hugepages_info.h"

char *TCID = "hugeshmget02";
int TST_TOTAL = 4;
unsigned long huge_pages_shm_to_be_allocated;

int exp_enos[] = {ENOENT, EEXIST, EINVAL, 0};	/* 0 terminated list of */
						/* expected errnos 	*/

int shm_id_1 = -1;
int shm_id_2 = -1;
int shmkey2;

struct test_case_t {
	int *skey;
	int size;
	int flags;
	int error;
};

int main(int ac, char **av) {
	int lc;				/* loop counter */
	char *msg;			/* message returned from parse_opts */
	int i;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

        if (get_no_of_hugepages() <= 0 || hugepages_size() <= 0)
             tst_brkm(TCONF, NULL, "Not enough available Hugepages");
        else
             huge_pages_shm_to_be_allocated = ( get_no_of_hugepages() * hugepages_size() * 1024) / 2 ;

        struct test_case_t TC[] = {
         /* EINVAL - size is 0 */
         {&shmkey2, 0, SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW, EINVAL},
         /* EINVAL - size is negative */
         //{&shmkey2, -1, SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW, EINVAL},
         /* EINVAL - size is larger than created segment */
         {&shmkey, huge_pages_shm_to_be_allocated * 2, SHM_HUGETLB | SHM_RW, EINVAL},
         /* EEXIST - the segment exists and IPC_CREAT | IPC_EXCL is given */
         {&shmkey, huge_pages_shm_to_be_allocated, SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW, EEXIST},
         /* ENOENT - no segment exists for the key and IPC_CREAT is not given */
         /* use shm_id_2 (-1) as the key */
         {&shm_id_2, huge_pages_shm_to_be_allocated, SHM_HUGETLB | SHM_RW, ENOENT}
        };

	setup();			/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* loop through the test cases */
		for (i=0; i<TST_TOTAL; i++) {
			/*
			 * Look for a failure ...
			 */

			TEST(shmget(*(TC[i].skey), TC[i].size, TC[i].flags));

			if (TEST_RETURN != -1) {
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

	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void
setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	/* get an IPC resource key */
	shmkey = getipckey();

	shmkey2 = shmkey + 1;

	if ((shm_id_1 = shmget(shmkey, huge_pages_shm_to_be_allocated, IPC_CREAT | IPC_EXCL | SHM_RW)) == -1) {
		tst_brkm(TBROK, cleanup, "couldn't create shared memory segment in setup()");
	}

}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void
cleanup(void)
{
	/* if it exists, remove the shared memory resource */
	rm_shm(shm_id_1);

	tst_rmdir();

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}