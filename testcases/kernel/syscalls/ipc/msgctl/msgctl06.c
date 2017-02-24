/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/* 11/06/2002	Port to LTP	dbarrera@us.ibm.com */
/* 12/03/2008   Fix concurrency issue     mfertre@irisa.fr */

/*
 * NAME
 *	msgctl06
 *
 * CALLS
 *	msgget(2) msgctl(2)
 *
 * ALGORITHM
 *	Get and manipulate a message queue.
 *
 * RESTRICTIONS
 *
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include "test.h"
#include "ipcmsg.h"

void setup();
void cleanup();

char *TCID = "msgctl06";
int TST_TOTAL = 1;

/*
 * msgctl3_t -- union of msgctl(2)'s possible argument # 3 types.
 */
typedef union msgctl3_u {
	struct msqid_ds *msq_ds;	/* pointer to msqid_ds struct */
	struct ipc_acl *msq_acl;	/* pointer ACL buff and size */
} msgctl3_t;

extern int local_flag;

int msqid, status;
struct msqid_ds buf;

int main(int argc, char *argv[])
{
	key_t key;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	key = getipckey();
	TEST(msgget(key, IPC_CREAT | IPC_EXCL));
	msqid = TEST_RETURN;
	if (TEST_RETURN == -1) {
		tst_brkm(TFAIL | TTERRNO, NULL, "msgget() failed");
	}

	TEST(msgctl(msqid, IPC_STAT, &buf));
	status = TEST_RETURN;
	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO,
			 "msgctl(msqid, IPC_STAT, &buf) failed");
		(void)msgctl(msqid, IPC_RMID, NULL);
		tst_exit();
	}

	/*
	 * Check contents of msqid_ds structure.
	 */

	if (buf.msg_qnum != 0) {
		tst_brkm(TFAIL, NULL, "error: unexpected nbr of messages %ld",
			 buf.msg_qnum);
	}
	if (buf.msg_perm.uid != getuid()) {
		tst_brkm(TFAIL, NULL, "error: unexpected uid %d",
			 buf.msg_perm.uid);
	}
	if (buf.msg_perm.gid != getgid()) {
		tst_brkm(TFAIL, NULL, "error: unexpected gid %d",
			 buf.msg_perm.gid);
	}
	if (buf.msg_perm.cuid != getuid()) {
		tst_brkm(TFAIL, NULL, "error: unexpected cuid %d",
			 buf.msg_perm.cuid);
	}
	if (buf.msg_perm.cgid != getgid()) {
		tst_brkm(TFAIL, NULL, "error: unexpected cgid %d",
			 buf.msg_perm.cgid);
	}

	tst_resm(TPASS, "msgctl06 ran successfully!");

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
}

void cleanup(void)
{
	int status;

	(void)msgctl(msqid, IPC_RMID, NULL);
	if ((status = msgctl(msqid, IPC_STAT, &buf)) != -1) {
		(void)msgctl(msqid, IPC_RMID, NULL);
		tst_resm(TFAIL, "msgctl(msqid, IPC_RMID) failed");

	}

	tst_rmdir();
}
