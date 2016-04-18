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

#include <pwd.h>
#include "test.h"
#include "safe_macros.h"
#include "mem.h"
#include "hugetlb.h"

char *TCID = "hugeshmctl02";
int TST_TOTAL = 4;

static size_t shm_size;
static int shm_id_1 = -1;
static int shm_id_2 = -1;
static int shm_id_3 = -1;
static struct shmid_ds buf;

static long hugepages = 128;
static option_t options[] = {
	{"s:", &sflag, &nr_opt},
	{NULL, NULL, NULL}
};

struct test_case_t {
	int *shmid;
	int cmd;
	struct shmid_ds *sbuf;
	int error;
} TC[] = {
	/* EFAULT - IPC_SET & buf isn't valid */
	{
	&shm_id_2, IPC_SET, (struct shmid_ds *)-1, EFAULT},
	    /* EFAULT - IPC_STAT & buf isn't valid */
	{
	&shm_id_2, IPC_STAT, (struct shmid_ds *)-1, EFAULT},
	    /* EINVAL - the shmid is not valid */
	{
	&shm_id_3, IPC_STAT, &buf, EINVAL},
	    /* EINVAL - the command is not valid */
	{
&shm_id_2, -1, &buf, EINVAL},};

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
			TEST(shmctl(*(TC[i].shmid), TC[i].cmd, TC[i].sbuf));
			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "shmctl succeeded "
					 "unexpectedly");
				continue;
			}
			if (TEST_ERRNO == TC[i].error)
				tst_resm(TPASS | TTERRNO, "shmctl failed "
					 "as expected");
			else
				tst_resm(TFAIL | TTERRNO, "shmctl failed "
					 "unexpectedly - expect errno = "
					 "%d, got", TC[i].error);
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

	/* create a shared memory segment without read or write permissions */
	shm_id_1 = shmget(shmkey, shm_size, SHM_HUGETLB | IPC_CREAT | IPC_EXCL);
	if (shm_id_1 == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget #1");

	/* create a shared memory segment with read and write permissions */
	shm_id_2 = shmget(shmkey + 1, shm_size,
			  SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
	if (shm_id_2 == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget #2");

	TEST_PAUSE;
}

void cleanup(void)
{
	rm_shm(shm_id_1);
	rm_shm(shm_id_2);

	set_sys_tune("nr_hugepages", orig_hugepages, 0);

	tst_rmdir();
}
