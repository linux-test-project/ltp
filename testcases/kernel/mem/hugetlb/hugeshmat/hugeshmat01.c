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
 *	hugeshmat01.c
 *
 * DESCRIPTION
 *	hugeshmat01 - test that shmat() works correctly
 *
 * ALGORITHM
 *	create a large shared memory resouce with read/write permissions
 *	loop if that option was specified
 *	call shmat() with the TEST() macro using three valid conditions
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing
 *		check for the correct conditions after the call
 *		if correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  hugeshmat01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

char *TCID = "hugeshmat01";
int TST_TOTAL = 3;

#define CASE0		10	/* values to write into the shared */
#define CASE1		20	/* memory location.                */

static size_t shm_size;
static int shm_id_1 = -1;
static void *addr;

static long hugepages = 128;
static option_t options[] = {
	{"s:", &sflag, &nr_opt},
	{NULL, NULL, NULL}
};

struct test_case_t {
	int *shmid;
	void *addr;
	int flags;
} TC[] = {
	/* a straight forward read/write attach */
	{
	&shm_id_1, 0, 0},
	    /*
	     * an attach using non aligned memory
	     * -1 will be replaced with an unaligned addr
	     */
	{
	&shm_id_1, (void *)-1, SHM_RND},
	    /* a read only attach */
	{
	&shm_id_1, 0, SHM_RDONLY}
};

static void check_functionality(int i);

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, options, NULL);

	if (sflag)
		hugepages = SAFE_STRTOL(NULL, nr_opt, 0, LONG_MAX);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			addr = shmat(*(TC[i].shmid), TC[i].addr, TC[i].flags);
			if (addr == (void *)-1) {
				tst_brkm(TFAIL | TERRNO, cleanup, "shmat");
			} else {
				check_functionality(i);
			}

			/*
			 * addr in TC[0] will be used to generate an unaligned
			 * address for TC[1]
			 */
			if (i == 0 && addr != (void *)-1)
				TC[1].addr = (void *)(((unsigned long)addr &
						       ~(SHMLBA - 1)) + SHMLBA -
						      1);
			if (shmdt(addr) == -1)
				tst_brkm(TBROK | TERRNO, cleanup, "shmdt");
		}
	}
	cleanup();
	tst_exit();
}

/*
 * check_functionality - check various conditions to make sure they
 *			 are correct.
 */
static void check_functionality(int i)
{
	void *orig_add;
	int *shared;
	struct shmid_ds buf;

	shared = (int *)addr;

	/* stat the shared memory ID */
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmctl");

	/* check the number of attaches */
	if (buf.shm_nattch != 1) {
		tst_resm(TFAIL, "# of attaches is incorrect");
		return;
	}

	/* check the size of the segment */
	if (buf.shm_segsz != shm_size) {
		tst_resm(TFAIL, "segment size is incorrect");
		return;
	}

	/* check for specific conditions depending on the type of attach */
	switch (i) {
	case 0:
		/*
		 * Check the functionality of the first call by simply
		 * "writing" a value to the shared memory space.
		 * If this fails the program will get a SIGSEGV, dump
		 * core and exit.
		 */
		*shared = CASE0;
		break;
	case 1:
		/*
		 * Check the functionality of the second call by writing
		 * a value to the shared memory space and then checking
		 * that the original address given was rounded down as
		 * specified in the man page.
		 */
		*shared = CASE1;
		orig_add = addr + ((unsigned long)TC[i].addr % SHMLBA);
		if (orig_add != TC[i].addr) {
			tst_resm(TFAIL, "shared memory address is not "
				 "correct");
			return;
		}
		break;
	case 2:
		/*
		 * This time the shared memory is read only.  Read the value
		 * and check that it is equal to the value set in case #2,
		 * because shared memory is persistent.
		 */
		if (*shared != CASE1) {
			tst_resm(TFAIL, "shared memory value isn't correct");
			return;
		}
		break;
	}
	tst_resm(TPASS, "conditions and functionality are correct");
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
	shm_id_1 = shmget(shmkey++, shm_size,
			  SHM_HUGETLB | SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id_1 == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget");

	TEST_PAUSE;
}

void cleanup(void)
{
	rm_shm(shm_id_1);

	set_sys_tune("nr_hugepages", orig_hugepages, 0);

	tst_rmdir();
}
