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
 *	hugeshmctl03.c
 *
 * DESCRIPTION
 *	hugeshmctl03 - check for EACCES, and EPERM errors
 *
 * ALGORITHM
 *	create a large shared memory segment with root only read & write
 *	permissions fork a child process
 *	if child
 *	  set the ID of the child process to that of "ltpuser1"
 *	  call do_child()
 *	  loop if that option was specified
 *	    call shmctl() using three different invalid cases
 *	    check the errno value
 *	      issue a PASS message if we get EACCES or EPERM
 *	    otherwise, the tests fails
 *	      issue a FAIL message
 *	  call cleanup
 *	if parent
 *	  wait for child to exit
 *	  remove the large shared memory segment
 *
 * USAGE:  <for command-line>
 *  hugeshmctl03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	test must be run as root
 */

#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "safe_macros.h"
#include "mem.h"
#include "hugetlb.h"

char *TCID = "hugeshmctl03";
int TST_TOTAL = 3;

static size_t shm_size;
static int shm_id_1 = -1;
static struct shmid_ds buf;
static uid_t ltp_uid;
static char *ltp_user = "nobody";

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
	/* EACCES - child has no read permission for segment */
	{
	&shm_id_1, IPC_STAT, &buf, EACCES},
	    /* EPERM - IPC_SET - child doesn't have permission to change segment */
	{
	&shm_id_1, IPC_SET, &buf, EPERM},
	    /* EPERM - IPC_RMID - child can not remove the segment */
	{
&shm_id_1, IPC_RMID, &buf, EPERM},};

static void do_child(void);

int main(int ac, char **av)
{
	pid_t pid;
	int status;

	tst_parse_opts(ac, av, options, NULL);

	if (sflag)
		hugepages = SAFE_STRTOL(NULL, nr_opt, 0, LONG_MAX);

	setup();

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
	case 0:
		/* set  the user ID of the child to the non root user */
		if (setuid(ltp_uid) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "setuid");
		do_child();
		tst_exit();
	default:
		if (waitpid(pid, &status, 0) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "waitpid");
	}
	cleanup();
	tst_exit();
}

static void do_child(void)
{
	int i, lc;

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
}

void setup(void)
{
	long hpage_size;

	tst_require_root();
	check_hugepage();
	tst_sig(FORK, DEF_HANDLER, cleanup);
	tst_tmpdir();

	orig_hugepages = get_sys_tune("nr_hugepages");
	set_sys_tune("nr_hugepages", hugepages, 1);
	hpage_size = read_meminfo("Hugepagesize:") * 1024;

	shm_size = hpage_size * hugepages / 2;
	update_shm_size(&shm_size);
	shmkey = getipckey(cleanup);
	shm_id_1 = shmget(shmkey, shm_size,
			  SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
	if (shm_id_1 == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget");

	/* get the userid for a non root user */
	ltp_uid = getuserid(cleanup, ltp_user);

	TEST_PAUSE;
}

void cleanup(void)
{
	rm_shm(shm_id_1);

	set_sys_tune("nr_hugepages", orig_hugepages, 0);

	tst_rmdir();
}
