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
 *	hugeshmat02.c
 *
 * DESCRIPTION
 *	hugeshmat02 - check for EINVAL and EACCES errors with hugetlb
 *
 * ALGORITHM
 *	loop if that option was specified
 *	  call shmat() using three invalid test cases
 *	  check the errno value
 *	    issue a PASS message if we get EINVAL or EACCES
 *	  otherwise, the tests fails
 *	    issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  hugeshmat02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated By Robbie Williamson
 *
 * RESTRICTIONS
 *	Must be ran as root
 */

#include <pwd.h>
#include "hugetlb.h"
#include "safe_macros.h"
#include "mem.h"

char *TCID = "hugeshmat02";
int TST_TOTAL = 2;

#if __WORDSIZE == 64
#define NADDR	0x10000000eef	/* a 64bit non alligned address value */
#else
#define NADDR	0x60000eef	/* a non alligned address value */
#endif

static size_t shm_size;
static int shm_id_1 = -1;
static int shm_id_2 = -1;
static void *addr;

static long hugepages = 128;
static option_t options[] = {
	{"s:", &sflag, &nr_opt},
	{NULL, NULL, NULL}
};

struct test_case_t {
	int *shmid;
	void *addr;
	int error;
} TC[] = {
	/* EINVAL - the shared memory ID is not valid */
	{
	&shm_id_1, NULL, EINVAL},
	    /* EINVAL - the address is not page aligned and SHM_RND is not given */
	{
&shm_id_2, (void *)NADDR, EINVAL},};

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
			addr = shmat(*(TC[i].shmid), TC[i].addr, 0);
			if (addr != (void *)-1) {
				tst_resm(TFAIL, "shmat suceeded unexpectedly");
				continue;
			}
			if (errno == TC[i].error)
				tst_resm(TPASS | TERRNO, "shmat failed as "
					 "expected");
			else
				tst_resm(TFAIL | TERRNO, "shmat failed "
					 "unexpectedly - expect errno=%d, "
					 "got", TC[i].error);
		}
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

	/* create a shared memory resource with read and write permissions */
	/* also post increment the shmkey for the next shmget call */
	shm_id_2 = shmget(shmkey++, shm_size,
			  SHM_HUGETLB | SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id_2 == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget");

	TEST_PAUSE;
}

void cleanup(void)
{
	rm_shm(shm_id_2);

	set_sys_tune("nr_hugepages", orig_hugepages, 0);

	tst_rmdir();
}
