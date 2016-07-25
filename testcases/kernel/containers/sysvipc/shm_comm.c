/* Copyright (c) 2014 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 2 the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 * File: shm_comm.c
 *
 * Description:
 * 1. Clones two child processes with CLONE_NEWIPC flag, each child
 *    allocates System V shared memory segment (shm) with the _identical_
 *    key and attaches that segment into its address space.
 * 2. Child1 writes into the shared memory segment.
 * 3. Child2 writes into the shared memory segment.
 * 4. Writes to the shared memory segment with the identical key but from
 *    two different IPC namespaces should not interfere with each other
 *    and so child1 checks whether its shared segment wasn't changed
 *    by child2, if it wasn't test passes, otherwise test fails.
 */

#define _GNU_SOURCE
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include "ipcns_helper.h"
#include "test.h"
#include "safe_macros.h"


#define TESTKEY 124426L
#define SHMSIZE 50
char *TCID	= "shm_comm";
int TST_TOTAL	= 1;

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	tst_require_root();
	check_newipc();
	tst_tmpdir();
	TST_CHECKPOINT_INIT(tst_rmdir);
}

int chld1_shm(void *arg)
{
	int id, rval = 0;
	char *shmem;

	id = shmget(TESTKEY, SHMSIZE, IPC_CREAT);
	if (id == -1) {
		perror("shmget");
		return 2;
	}

	if ((shmem = shmat(id, NULL, 0)) == (char *) -1) {
		perror("shmat");
		shmctl(id, IPC_RMID, NULL);
		return 2;
	}

	*shmem = 'A';

	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	/* if child1 shared segment has changed (by child2) report fail */
	if (*shmem != 'A')
		rval = 1;

	/* tell child2 to continue */
	TST_SAFE_CHECKPOINT_WAKE(NULL, 0);

	shmdt(shmem);
	shmctl(id, IPC_RMID, NULL);
	return rval;
}

int chld2_shm(void *arg)
{
	int id;
	char *shmem;

	id = shmget(TESTKEY, SHMSIZE, IPC_CREAT);
	if (id == -1) {
		perror("shmget");
		return 2;
	}

	if ((shmem = shmat(id, NULL, 0)) == (char *) -1) {
		perror("shmat");
		shmctl(id, IPC_RMID, NULL);
		return 2;
	}

	/* wait for child1 to write to his segment */
	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);

	*shmem = 'B';

	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	shmdt(shmem);
	shmctl(id, IPC_RMID, NULL);
	return 0;
}

static void test(void)
{
	int status, ret = 0;

	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld1_shm, NULL);
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");

	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld2_shm, NULL);
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");


	while (wait(&status) > 0) {
		if (WIFEXITED(status) && WEXITSTATUS(status) == 1)
			ret = 1;
		if (WIFEXITED(status) && WEXITSTATUS(status) == 2)
			tst_brkm(TBROK | TERRNO, cleanup, "error in child");
		if (WIFSIGNALED(status)) {
			tst_resm(TFAIL, "child was killed with signal %s",
					tst_strsig(WTERMSIG(status)));
			return;
		}
	}

	if (ret)
		tst_resm(TFAIL, "SysV shm: communication with identical keys"
				" between namespaces");
	else
		tst_resm(TPASS, "SysV shm: communication with identical keys"
				" between namespaces");
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++)
		test();

	cleanup();
	tst_exit();
}
