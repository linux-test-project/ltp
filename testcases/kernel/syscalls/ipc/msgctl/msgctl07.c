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
/* 11/06/2002   Port to LTP     dbarrera@us.ibm.com */
/* 12/03/2008   Fix concurrency issue     mfertre@irisa.fr */

/*
 * NAME
 *	msgctl07
 *
 * CALLS
 *	msgget(2) msgctl(2) msgop(2)
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
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include "test.h"
#include "ipcmsg.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

typedef void (*sighandler_t) (int);
volatile int ready;

#define BYTES 100
#define SECS 10

void setup();
void cleanup();
void do_child_1();
void do_child_2();

char *TCID = "msgctl07";
int TST_TOTAL = 1;

/* Used by main() and do_child_1(): */
static int msqid;
struct my_msgbuf {
	long type;
	char text[BYTES];
} p1_msgp, p2_msgp, p3_msgp, c1_msgp, c2_msgp, c3_msgp;

int main(int argc, char *argv[])
{
	key_t key;
	int pid, status;
	int i, j, k;
	sighandler_t alrm();

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	key = getipckey();
	if ((msqid = msgget(key, IPC_CREAT | IPC_EXCL)) == -1) {
		tst_brkm(TFAIL | TERRNO, NULL, "msgget() failed");

	}

	pid = FORK_OR_VFORK();
	if (pid < 0) {
		(void)msgctl(msqid, IPC_RMID, NULL);
		tst_brkm(TFAIL, NULL,
			 "\tFork failed (may be OK if under stress)");
	} else if (pid == 0) {
		do_child_1();
	} else {
		struct sigaction act;

		memset(&act, 0, sizeof(act));
		act.sa_handler = (sighandler_t) alrm;
		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, SIGALRM);
		if ((sigaction(SIGALRM, &act, NULL)) < 0) {
			tst_resm(TFAIL | TERRNO, "signal failed");
			kill(pid, SIGKILL);
			(void)msgctl(msqid, IPC_RMID, NULL);
			tst_exit();
		}
		ready = 0;
		alarm(SECS);
		while (!ready)	/* make the child wait */
			usleep(50000);
		for (i = 0; i < BYTES; i++)
			p1_msgp.text[i] = 'i';
		p1_msgp.type = 1;
		if (msgsnd(msqid, &p1_msgp, BYTES, 0) == -1) {
			tst_resm(TFAIL | TERRNO, "msgsnd() failed");
			kill(pid, SIGKILL);
			(void)msgctl(msqid, IPC_RMID, NULL);
			tst_exit();
		}
		wait(&status);
	}
	if ((status >> 8) == 1) {
		tst_brkm(TFAIL, NULL, "test failed. status = %d",
			 (status >> 8));
	}

	pid = FORK_OR_VFORK();
	if (pid < 0) {
		(void)msgctl(msqid, IPC_RMID, NULL);
		tst_brkm(TFAIL, NULL,
			 "\tFork failed (may be OK if under stress)");
	} else if (pid == 0) {
		do_child_2();
	} else {
		struct sigaction act;

		memset(&act, 0, sizeof(act));
		act.sa_handler = (sighandler_t) alrm;
		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, SIGALRM);
		if ((sigaction(SIGALRM, &act, NULL)) < 0) {
			tst_resm(TFAIL | TERRNO, "signal failed");
			kill(pid, SIGKILL);
			(void)msgctl(msqid, IPC_RMID, NULL);
			tst_exit();
		}
		ready = 0;
		alarm(SECS);
		while (!ready)	/* make the child wait */
			usleep(50000);
		for (i = 0; i < BYTES; i++)
			p1_msgp.text[i] = 'i';
		p1_msgp.type = 1;
		if (msgsnd(msqid, &p1_msgp, BYTES, 0) == -1) {
			tst_resm(TFAIL | TERRNO, "msgsnd() failed");
			kill(pid, SIGKILL);
			(void)msgctl(msqid, IPC_RMID, NULL);
			tst_exit();
		}
		for (j = 0; j < BYTES; j++)
			p2_msgp.text[j] = 'j';
		p2_msgp.type = 2;
		if (msgsnd(msqid, &p2_msgp, BYTES, 0) == -1) {
			tst_resm(TFAIL | TERRNO, "msgsnd() failed");
			kill(pid, SIGKILL);
			(void)msgctl(msqid, IPC_RMID, NULL);
			tst_exit();
		}
		for (k = 0; k < BYTES; k++)
			p3_msgp.text[k] = 'k';
		p3_msgp.type = 3;
		if (msgsnd(msqid, &p3_msgp, BYTES, 0) == -1) {
			tst_resm(TFAIL | TERRNO, "msgsnd() failed");
			kill(pid, SIGKILL);
			(void)msgctl(msqid, IPC_RMID, NULL);
			tst_exit();
		}
		wait(&status);
	}
	if ((status >> 8) == 1) {
		tst_brkm(TFAIL, NULL, "test failed. status = %d",
			 (status >> 8));
	}
	/*
	 * Remove the message queue from the system
	 */
#ifdef DEBUG
	tst_resm(TINFO, "Removing the message queue");
#endif
	fflush(stdout);
	(void)msgctl(msqid, IPC_RMID, NULL);
	if ((status = msgctl(msqid, IPC_STAT, NULL)) != -1) {
		(void)msgctl(msqid, IPC_RMID, NULL);
		tst_brkm(TFAIL, NULL, "msgctl(msqid, IPC_RMID) failed");

	}

	fflush(stdout);
	tst_resm(TPASS, "msgctl07 ran successfully!");

	cleanup();

	tst_exit();
}

sighandler_t alrm(int sig LTP_ATTRIBUTE_UNUSED)
{
	ready++;
	return 0;
}

void do_child_1(void)
{
	int i;
	int size;

	if ((size = msgrcv(msqid, &c1_msgp, BYTES, 0, 0)) == -1) {
		tst_brkm(TFAIL | TERRNO, NULL, "msgrcv() failed");
	}
	if (size != BYTES) {
		tst_brkm(TFAIL, NULL, "error: received %d bytes expected %d",
			 size,
			 BYTES);
	}
	for (i = 0; i < BYTES; i++)
		if (c1_msgp.text[i] != 'i') {
			tst_brkm(TFAIL, NULL, "error: corrup message");
		}
	exit(0);
}

void do_child_2(void)
{
	int i, j, k;
	int size;

	if ((size = msgrcv(msqid, &c3_msgp, BYTES, 3, 0)) == -1) {
		tst_brkm(TFAIL | TERRNO, NULL, "msgrcv() failed");
	}
	if (size != BYTES) {
		tst_brkm(TFAIL, NULL, "error: received %d bytes expected %d",
			 size,
			 BYTES);
	}
	for (k = 0; k < BYTES; k++)
		if (c3_msgp.text[k] != 'k') {
			tst_brkm(TFAIL, NULL, "error: corrupt message");
		}
	if ((size = msgrcv(msqid, &c2_msgp, BYTES, 2, 0)) == -1) {
		tst_brkm(TFAIL | TERRNO, NULL, "msgrcv() failed");
	}
	if (size != BYTES) {
		tst_brkm(TFAIL, NULL, "error: received %d bytes expected %d",
			 size,
			 BYTES);
	}
	for (j = 0; j < BYTES; j++)
		if (c2_msgp.text[j] != 'j') {
			tst_brkm(TFAIL, NULL, "error: corrupt message");
		}
	if ((size = msgrcv(msqid, &c1_msgp, BYTES, 1, 0)) == -1) {
		tst_brkm(TFAIL | TERRNO, NULL, "msgrcv() failed");
	}
	if (size != BYTES) {
		tst_brkm(TFAIL, NULL, "error: received %d bytes expected %d",
			 size,
			 BYTES);
	}
	for (i = 0; i < BYTES; i++)
		if (c1_msgp.text[i] != 'i') {
			tst_brkm(TFAIL, NULL, "error: corrupt message");
		}

	exit(0);
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();
}

void cleanup(void)
{
	tst_rmdir();
}
