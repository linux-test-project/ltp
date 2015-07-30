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
 *	shmat02.c
 *
 * DESCRIPTION
 *	shmat02 - check for EINVAL and EACCES errors
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
 *  shmat02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *      27/02/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *      - Fix concurrency issue. The second key used for this test could
 *        conflict with the key from another task.
 *
 * RESTRICTIONS
 *	Must be ran as non-root
 */

#include "ipcshm.h"
#include <pwd.h>
#include "shmat_common.h"

char *TCID = "shmat02";
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int shm_id_1 = -1;
int shm_id_2 = -1;
int shm_id_3 = -1;

void *base_addr;		/* By probing this address first, we can make
				 * non-aligned addresses from it for different
				 * architectures without explicitly code it.
				 */

void *addr;			/* for result of shmat-call */

struct test_case_t {
	int *shmid;
	int offset;
	int error;
};

int TST_TOTAL = 3;

static void setup_tc(int i, struct test_case_t *tc)
{

	struct test_case_t TC[] = {
		/* EINVAL - the shared memory ID is not valid */
		{&shm_id_1, 0, EINVAL},
		/* EINVAL - the address is not page aligned and SHM_RND is not given */
		{&shm_id_2, SHMLBA - 1, EINVAL},
		/* EACCES - the shared memory resource has no read/write permission */
		{&shm_id_3, 0, EACCES}
	};

	if (i > TST_TOTAL || i < 0)
		return;

	*tc = TC[i];
}

int main(int ac, char **av)
{
	int lc;
	int i;
	struct test_case_t *tc;

	tc = NULL;

	tst_parse_opts(ac, av, NULL, NULL);

	tc = malloc(sizeof(struct test_case_t));
	if (tc == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "malloc failed");

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			setup_tc(i, tc);

			base_addr = probe_free_addr();
			errno = 0;
			addr = shmat(*(tc->shmid), base_addr + tc->offset, 0);

			if (addr != (void *)-1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (errno == tc->error)
				tst_resm(TPASS | TERRNO,
					 "shmat failed as expected");
			else
				tst_resm(TFAIL,
					 "shmat failed unexpectedly; expected: "
					 "%d - %s", tc->error,
					 strerror(tc->error));
		}
	}

	cleanup();

	tst_exit();
}

void setup(void)
{
	key_t shmkey2;

	tst_require_root();
	ltpuser = getpwnam(nobody_uid);
	if (ltpuser == NULL)
		tst_brkm(TBROK | TERRNO, NULL, "getpwnam failed");
	if (setuid(ltpuser->pw_uid) == -1)
		tst_brkm(TBROK | TERRNO, NULL, "setuid failed");

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	shmkey = getipckey();

	shm_id_2 = shmget(shmkey, INT_SIZE, SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id_2 == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget #1 failed");

	/* Get an new IPC resource key. */
	shmkey2 = getipckey();

	/* create a shared memory resource without read and write permissions */
	shm_id_3 = shmget(shmkey2, INT_SIZE, IPC_CREAT | IPC_EXCL);
	if (shm_id_3 == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget #2 failed");
}

void cleanup(void)
{
	/* if they exist, remove the shared memory resources */
	rm_shm(shm_id_2);
	rm_shm(shm_id_3);

	tst_rmdir();

}
