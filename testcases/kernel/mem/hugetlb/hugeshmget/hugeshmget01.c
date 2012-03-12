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
 *	hugeshmget01.c
 *
 * DESCRIPTION
 *	hugeshmget01 - test that shmget() correctly creates a large
 *			shared memory segment
 *
 * ALGORITHM
 *	loop if that option was specified
 *	use the TEST() macro to call shmget()
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing
 *		stat the shared memory resource
 *		check the size, creator pid and mode
 *		if correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	  else issue a PASS message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  shmget01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
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

char *TCID = "hugeshmget01";
int TST_TOTAL = 1;

static int shm_id_1 = -1;

int main(int ac, char **av)
{
	int lc;
	char *msg;
	struct shmid_ds buf;
	size_t shm_size;

	shm_size = 0;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (get_no_of_hugepages() <= 0 || hugepages_size() <= 0)
		tst_brkm(TCONF, NULL, "Not enough available Hugepages");
	else
		shm_size = (get_no_of_hugepages()*hugepages_size()*1024) / 2;

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		shm_id_1 = shmget(shmkey, shm_size,
			    SHM_HUGETLB|IPC_CREAT|IPC_EXCL|SHM_RW);
		if (shm_id_1 == -1) {
			tst_resm(TFAIL|TERRNO, "shmget");
		} else {
			if (STD_FUNCTIONAL_TEST) {
				/* do a STAT and check some info */
				if (shmctl(shm_id_1, IPC_STAT, &buf) == -1) {
					tst_resm(TBROK|TERRNO,
						    "shmctl(IPC_STAT)");
					continue;
				}
				/* check the seqment size */
				if (buf.shm_segsz != shm_size) {
					tst_resm(TFAIL, "seqment size is not "
							"correct");
					continue;
				}
				/* check the pid of the creator */
				if (buf.shm_cpid != getpid()) {
					tst_resm(TFAIL, "creator pid is not "
							"correct");
					continue;
				}
				/*
				 * check the mode of the seqment
				 * mask out all but the lower 9 bits
				 */
				if ((buf.shm_perm.mode & MODE_MASK) !=
					    ((SHM_RW) & MODE_MASK)) {
					tst_resm(TFAIL, "segment mode is not "
							"correct");
					continue;
				}
				/* if we get here, everything looks good */
				tst_resm(TPASS, "size, pid & mode are correct");
			} else {
				tst_resm(TPASS, "call succeeded");
			}
		}

		/*
		 * clean up things in case we are looping
		 */
		if (shmctl(shm_id_1, IPC_RMID, NULL) == -1)
			tst_resm(TBROK|TERRNO, "shmctl(IPC_RMID)");
		else
			shm_id_1 = -1;
	}
	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	tst_tmpdir();

	shmkey = getipckey();

	TEST_PAUSE;
}

void cleanup(void)
{
	TEST_CLEANUP;

	rm_shm(shm_id_1);

	tst_rmdir();
}
