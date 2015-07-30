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
 * DESCRIPTION
 *	semop02 - test for E2BIG, EACCES, EFAULT, EINVAL and ERANGE errors
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 *      10/03/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *      - Fix concurrency issue. The second key used for this test could
 *        conflict with the key from another task.
 */

#define _GNU_SOURCE
#include <pwd.h>
#include "test.h"
#include "safe_macros.h"
#include "ipcsem.h"

char *TCID = "semop02";

static void semop_verify(int i);
int sem_id_1 = -1;	/* a semaphore set with read & alter permissions */
int sem_id_2 = -1;	/* a semaphore set without read & alter permissions */
int bad_id = -1;

struct sembuf s_buf[PSEMS];

int badbuf = -1;

#define NSOPS	5		/* a resonable number of operations */
#define	BIGOPS	1024		/* a value that is too large for the number */
				/* of semop operations that are permitted   */
struct test_case_t {
	int *semid;
	struct sembuf *t_sbuf;
	unsigned t_ops;
	int error;
} TC[] = {
	{&sem_id_1, (struct sembuf *)&s_buf, BIGOPS, E2BIG},
	{&sem_id_2, (struct sembuf *)&s_buf, NSOPS, EACCES},
	{&sem_id_1, (struct sembuf *)-1, NSOPS, EFAULT},
	{&sem_id_1, (struct sembuf *)&s_buf, 0, EINVAL},
	{&bad_id, (struct sembuf *)&s_buf, NSOPS, EINVAL},
	{&sem_id_1, (struct sembuf *)&s_buf, 1, ERANGE}
};

int TST_TOTAL = ARRAY_SIZE(TC);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			semop_verify(i);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	char nobody_uid[] = "nobody";
	struct passwd *ltpuser;
	key_t semkey2;
	struct seminfo ipc_buf;
	union semun arr;

	tst_require_root();

	ltpuser = SAFE_GETPWNAM(NULL, nobody_uid);
	SAFE_SETUID(NULL, ltpuser->pw_uid);

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* get an IPC resource key */
	semkey = getipckey();

	/* create a semaphore set with read and alter permissions */
	sem_id_1 = semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA);
	if (sem_id_1 == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "couldn't create semaphore in setup");
	}

	/* Get an new IPC resource key. */
	semkey2 = getipckey();

	/* create a semaphore set without read and alter permissions */
	sem_id_2 = semget(semkey2, PSEMS, IPC_CREAT | IPC_EXCL);
	if (sem_id_2 == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "couldn't create semaphore in setup");
	}

	arr.__buf = &ipc_buf;
	if (semctl(sem_id_1, 0, IPC_INFO, arr) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "semctl() IPC_INFO failed");

	/* for ERANGE errno test */
	arr.val = 1;
	s_buf[0].sem_op = ipc_buf.semvmx;
	if (semctl(sem_id_1, 0, SETVAL, arr) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "semctl() SETVAL failed");
}

static void semop_verify(int i)
{
	TEST(semop(*(TC[i].semid), TC[i].t_sbuf, TC[i].t_ops));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == TC[i].error) {
		tst_resm(TPASS | TTERRNO, "semop failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "semop failed unexpectedly; expected: "
			 "%d - %s", TC[i].error, strerror(TC[i].error));
	}
}

void cleanup(void)
{
	/* if they exist, remove the semaphore resources */
	rm_sema(sem_id_1);
	rm_sema(sem_id_2);

	tst_rmdir();
}
