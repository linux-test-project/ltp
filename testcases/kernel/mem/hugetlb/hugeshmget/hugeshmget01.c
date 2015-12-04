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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include "hugetlb.h"
#include "safe_macros.h"
#include "mem.h"

char *TCID = "hugeshmget01";
int TST_TOTAL = 1;

static size_t shm_size;
static int shm_id_1 = -1;

static long hugepages = 128;
static option_t options[] = {
	{"s:", &sflag, &nr_opt},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;
	struct shmid_ds buf;

	tst_parse_opts(ac, av, options, NULL);

	if (sflag)
		hugepages = SAFE_STRTOL(NULL, nr_opt, 0, LONG_MAX);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		shm_id_1 = shmget(shmkey, shm_size,
				  SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
		if (shm_id_1 == -1) {
			tst_resm(TFAIL | TERRNO, "shmget");
		} else {
			/* do a STAT and check some info */
			if (shmctl(shm_id_1, IPC_STAT, &buf) == -1) {
				tst_resm(TBROK | TERRNO,
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
		}

		/*
		 * clean up things in case we are looping
		 */
		if (shmctl(shm_id_1, IPC_RMID, NULL) == -1)
			tst_resm(TBROK | TERRNO, "shmctl(IPC_RMID)");
		else
			shm_id_1 = -1;
	}
	cleanup();
	tst_exit();
}

void setup(void)
{
	long hpage_size;

	tst_require_root();
	check_hugepage();
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	tst_tmpdir();

	orig_hugepages = get_sys_tune("nr_hugepages");
	set_sys_tune("nr_hugepages", hugepages, 1);
	hpage_size = read_meminfo("Hugepagesize:") * 1024;

	shm_size = hpage_size * hugepages / 2;
	update_shm_size(&shm_size);
	shmkey = getipckey(cleanup);

	TEST_PAUSE;
}

void cleanup(void)
{
	rm_shm(shm_id_1);

	set_sys_tune("nr_hugepages", orig_hugepages, 0);

	tst_rmdir();
}
