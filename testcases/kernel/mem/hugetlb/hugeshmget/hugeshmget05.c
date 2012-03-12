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
 *	hugeshmget05.c
 *
 * DESCRIPTION
 *	hugeshmget05 - test for EACCES error
 *
 * ALGORITHM
 *	create a large shared memory segment with root only read & write
 *		permissions
 *	fork a child process
 *	if child
 *	  set the ID of the child process to that of "nobody"
 *	  loop if that option was specified
 *	    call shmget() using the TEST() macro
 *	    check the errno value
 *	      issue a PASS message if we get EACCES
 *	    otherwise, the tests fails
 *	      issue a FAIL message
 *	  call cleanup
 *	if parent
 *	  wait for child to exit
 *	  remove the shared memory segment
 *
 * USAGE:  <for command-line>
 *  hugeshmget05 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	test must be run at root
 */

#include <sys/types.h>
#include <sys/wait.h>
#include "ipcshm.h"
#include "system_specific_hugepages_info.h"

char *TCID = "hugeshmget05";
int TST_TOTAL = 1;

static size_t shm_size;
static int shm_id_1 = -1;
static uid_t ltp_uid;
static char *ltp_user = "nobody";

static void do_child(void);

int main(int ac, char **av)
{
	char *msg;
	pid_t pid;
	int status;

	msg = parse_opts(ac, av, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (get_no_of_hugepages() <= 0 || hugepages_size() <= 0)
		tst_brkm(TCONF, cleanup, "Not enough available Hugepages");
	else
		shm_size = (get_no_of_hugepages()*hugepages_size()*1024) / 2;

	setup();

	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK|TERRNO, cleanup, "fork");
	case 0:
		/* set the user ID of the child to the non root user */
		if (setuid(ltp_uid) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "setuid");
		do_child();
		tst_exit();
	default:
		/* wait for the child to return */
		if (waitpid(pid, &status, 0) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "waitpid");
	}
	cleanup();
	tst_exit();
}

static void do_child(void)
{
	int lc;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		TEST(shmget(shmkey, shm_size, SHM_HUGETLB|SHM_RW));
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "shmget succeeded unexpectedly");
			continue;
		}
		if (TEST_ERRNO == EACCES)
			tst_resm(TPASS|TTERRNO, "shmget failed as expected");
		else
			tst_resm(TFAIL|TTERRNO, "shmget failed unexpectedly "
				    "- expect errno=EACCES, got");
	}
}

void setup(void)
{
	tst_require_root(NULL);
	tst_sig(FORK, DEF_HANDLER, cleanup);
	tst_tmpdir();

	shmkey = getipckey();
	shm_id_1 = shmget(shmkey, shm_size,
		    SHM_HUGETLB|SHM_RW|IPC_CREAT|IPC_EXCL);
	if (shm_id_1 == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "shmget #setup");

	/* get the userid for a non-root user */
	ltp_uid = getuserid(ltp_user);

	TEST_PAUSE;
}

void cleanup(void)
{
	TEST_CLEANUP;

	rm_shm(shm_id_1);

	tst_rmdir();
}
