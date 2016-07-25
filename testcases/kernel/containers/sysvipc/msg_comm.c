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
 * File: msg_comm.c
 *
 * Description:
 * 1. Clones two child processes with CLONE_NEWIPC flag, each child
 *    gets System V message queue (msg) with the _identical_ key.
 * 2. Child1 appends a message with identifier #1 to the message queue.
 * 3. Child2 appends a message with identifier #2 to the message queue.
 * 4. Appends to the message queue with the identical key but from
 *    two different IPC namespaces should not interfere with each other
 *    and so child1 checks whether its message queue doesn't contain
 *    a message with identifier #2, if it doesn't test passes, otherwise
 *    test fails.
 */

#define _GNU_SOURCE
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include "ipcns_helper.h"
#include "test.h"
#include "safe_macros.h"

#define TESTKEY 124426L
#define MSGSIZE 50
char *TCID	= "msg_comm";
int TST_TOTAL	= 1;

struct sysv_msg {
	long mtype;
	char mtext[MSGSIZE];
};

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

int chld1_msg(void *arg)
{
	int id, n, rval = 0;
	struct sysv_msg m;
	struct sysv_msg rec;

	id = msgget(TESTKEY, IPC_CREAT | 0600);
	if (id == -1) {
		perror("msgget");
		return 2;
	}

	m.mtype = 1;
	m.mtext[0] = 'A';
	if (msgsnd(id, &m, sizeof(struct sysv_msg) - sizeof(long), 0) == -1) {
		perror("msgsnd");
		msgctl(id, IPC_RMID, NULL);
		return 2;
	}

	/* wait for child2 to write into the message queue */
	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);

	/* if child1 message queue has changed (by child2) report fail */
	n = msgrcv(id, &rec, sizeof(struct sysv_msg) - sizeof(long),
		   2, IPC_NOWAIT);
	if (n == -1 && errno != ENOMSG) {
		perror("msgrcv");
		msgctl(id, IPC_RMID, NULL);
		return 2;
	}
	/* if mtype #2 was found in the message queue, it is fail */
	if (n > 0) {
		rval = 1;
	}

	/* tell child2 to continue */
	TST_SAFE_CHECKPOINT_WAKE(NULL, 0);

	msgctl(id, IPC_RMID, NULL);
	return rval;
}

int chld2_msg(void *arg)
{
	int id;
	struct sysv_msg m;

	id = msgget(TESTKEY, IPC_CREAT | 0600);
	if (id == -1) {
		perror("msgget");
		return 2;
	}

	m.mtype = 2;
	m.mtext[0] = 'B';
	if (msgsnd(id, &m, sizeof(struct sysv_msg) - sizeof(long), 0) == -1) {
		perror("msgsnd");
		msgctl(id, IPC_RMID, NULL);
		return 2;
	}

	/* tell child1 to continue and wait for it */
	TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(NULL, 0);

	msgctl(id, IPC_RMID, NULL);
	return 0;
}

static void test(void)
{
	int status, ret = 0;

	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld1_msg, NULL);
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");

	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWIPC, chld2_msg, NULL);
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
		tst_resm(TFAIL, "SysV msg: communication with identical keys"
				" between namespaces");
	else
		tst_resm(TPASS, "SysV msg: communication with identical keys"
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
