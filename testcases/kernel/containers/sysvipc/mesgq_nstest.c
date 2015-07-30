/* *************************************************************************
* Copyright (c) International Business Machines Corp., 2009
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
* Author: Veerendra C <vechandr@in.ibm.com>
*
* In Parent Process , create mesgq with key 154326L
* Now create container by passing 1 of the flag values..
*	Flag = clone, clone(CLONE_NEWIPC), or unshare(CLONE_NEWIPC)
* In cloned process, try to access the created mesgq
* Test PASS: If the mesgq is readable when flag is None.
* Test FAIL: If the mesgq is readable when flag is Unshare or Clone.
***************************************************************************/

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <libclone.h>
#include "test.h"
#include "ipcns_helper.h"

#define KEY_VAL		154326L
#define UNSHARESTR	"unshare"
#define CLONESTR	"clone"
#define NONESTR		"none"

char *TCID = "mesgq_nstest";
int TST_TOTAL = 1;
int p1[2];
int p2[2];
struct msg_buf {
	long int mtype;		/* type of received/sent message */
	char mtext[80];		/* text of the message */
} msg;

void mesgq_read(int id)
{
	int READMAX = 80;
	int n;
	/* read msg type 5 on the Q; msgtype, flags are last 2 params.. */

	n = msgrcv(id, &msg, READMAX, 5, 0);
	if (n == -1)
		perror("msgrcv"), tst_exit();

	tst_resm(TINFO, "Mesg read of %d bytes; Type %ld: Msg: %.*s",
		 n, msg.mtype, n, msg.mtext);
}

int check_mesgq(void *vtest)
{
	char buf[3];
	int id;

	(void) vtest;

	close(p1[1]);
	close(p2[0]);

	read(p1[0], buf, 3);
	id = msgget(KEY_VAL, 0);
	if (id == -1)
		write(p2[1], "notfnd", 7);
	else {
		write(p2[1], "exists", 7);
		mesgq_read(id);
	}
	tst_exit();
}

static void setup(void)
{
	tst_require_root();
	check_newipc();
}

int main(int argc, char *argv[])
{
	int ret, use_clone = T_NONE, id, n;
	char *tsttype = NONESTR;
	char buf[7];

	setup();

	if (argc != 2) {
		tst_resm(TFAIL, "Usage: %s <clone|unshare|none>", argv[0]);
		tst_brkm(TFAIL, NULL, " where clone, unshare, or fork specifies"
			 " unshare method.");
	}

	/* Using PIPE's to sync between container and Parent */
	if (pipe(p1) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	if (pipe(p2) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	tsttype = NONESTR;

	if (strcmp(argv[1], "clone") == 0) {
		use_clone = T_CLONE;
		tsttype = CLONESTR;
	} else if (strcmp(argv[1], "unshare") == 0) {
		use_clone = T_UNSHARE;
		tsttype = UNSHARESTR;
	}

	id = msgget(KEY_VAL, IPC_CREAT | IPC_EXCL | 0600);
	if (id == -1) {
		perror("msgget");
		/* Retry without attempting to create the MQ */
		id = msgget(KEY_VAL, 0);
		if (id == -1)
			perror("msgget failure"), exit(1);
	}

	msg.mtype = 5;
	strcpy(msg.mtext, "Message of type 5!");
	n = msgsnd(id, &msg, strlen(msg.mtext), 0);
	if (n == -1)
		perror("msgsnd"), tst_exit();

	tst_resm(TINFO, "mesgq namespaces test : %s", tsttype);
	/* fire off the test */
	ret = do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_mesgq, NULL);
	if (ret < 0) {
		tst_brkm(TFAIL, NULL, "%s failed", tsttype);
	}

	close(p1[0]);
	close(p2[1]);
	write(p1[1], "go", 3);

	read(p2[0], buf, 7);
	if (strcmp(buf, "exists") == 0) {
		if (use_clone == T_NONE)
			tst_resm(TPASS, "Plain cloned process found mesgq "
				 "inside container");
		else
			tst_resm(TFAIL,
				 "%s: Container init process found mesgq",
				 tsttype);
	} else {
		if (use_clone == T_NONE)
			tst_resm(TFAIL,
				 "Plain cloned process didn't find mesgq");
		else
			tst_resm(TPASS, "%s: Container didn't find mesgq",
				 tsttype);
	}

	/* Delete the mesgQ */
	id = msgget(KEY_VAL, 0);
	msgctl(id, IPC_RMID, NULL);

	tst_exit();
}
