/*
* Copyright (c) Bull S.A.S. 2008
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
***************************************************************************
* File: pidns30.c
*
*   Description:
*    This testcase checks if the si_pid is correctly set when a process
*    that has registered for notification on a posix mqueue is in a
*    descendant namespace wrt the process that sends a message to that posix
*    mqueue.
*
*   Test Assertion & Strategy:
*    Parent                                   Child
*    --------------------------------------------------------------------------
*    Create a POSIX mqueue.
*    Create a PID namespace container.
*                                             Open that mqueue for reading
*                                             Register for notification when a
*                                                message arrives in that mqueue
*                                             Install a handler for SIGUSR1.
*    Write something to the mqueue.
*                                             Inside the handler, check that
*                                                si_pid is set to 0
*
*   Usage: <for command-line>
*    pidns30
*
*   History:
*    DATE      NAME                             DESCRIPTION
*    01/12/08  Nadia Derbey               Creation of this test.
*              <Nadia.Derbey@bull.net>
*
******************************************************************************/
#define _GNU_SOURCE 1
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <mqueue.h>
#include "lapi/syscalls.h"
#include "pidns_helper.h"
#include "test.h"

char *TCID = "pidns30";
int TST_TOTAL = 1;

char *mqname = "mq1";
int result = TFAIL;

int father_to_child[2];
int child_to_father[2];

#define CHILD_PID       1
#define PARENT_PID      0

#define MSG      "HOW ARE YOU"
#define MSG_PRIO 1

#define NO_STEP	-1
#define F_STEP_0 0x00
#define F_STEP_1 0x01
#define F_STEP_2 0x02
#define F_STEP_3 0x03
#define C_STEP_0 0x10
#define C_STEP_1 0x11
#define C_STEP_2 0x12

mqd_t rc = -1;
mqd_t mqd = -1;

static void remove_pipe(int *fd)
{
	close(fd[0]);
	close(fd[1]);
}

static void remove_mqueue(mqd_t mqd)
{
	mq_close(mqd);
	tst_syscall(__NR_mq_unlink, mqname);
}

static void cleanup(void)
{
	if (mqd != -1) {
		remove_mqueue(mqd);
	}
	if (rc != -1) {
		remove_mqueue(rc);
	}
	remove_pipe(father_to_child);
	remove_pipe(child_to_father);
}

static void cleanup_child(void)
{
	if (mqd != -1) {
		tst_syscall(__NR_mq_notify, mqd, NULL);
	}
	cleanup();
}

/*
 * child_signal_handler() - to handle SIGUSR1
 *
 * XXX (garrcoop): add calls to cleanup_child() -- or should this be handled
 * from the libltp signal handler?
 */
static void child_signal_handler(int sig, siginfo_t * si, void *unused)
{
	char buf[256];
	struct mq_attr attr;

	if (si->si_signo != SIGUSR1) {
		printf("received signal = %d unexpectedly\n", si->si_signo);
		return;
	}

	if (si->si_code != SI_MESGQ) {
		printf("expected signal code SI_MESGQ; got %d instead\n",
		       si->si_code);
		return;
	}

	if (si->si_pid) {
		printf("expected signal originator PID = 0; got %d instead\n",
		       si->si_pid);
		return;
	} else {
		printf("signal originator PID = 0\n");
		result = TPASS;
	}

	/*
	 * Now read the message - Be silent on errors since this is not the
	 * test purpose.
	 */
	rc = mq_getattr(si->si_int, &attr);
	if (rc != -1)
		mq_receive(si->si_int, buf, attr.mq_msgsize, NULL);
}

/*
 * child_fn() - Inside container
 *
 * XXX (garrcoop): add more calls to cleanup_child()?
 */
int child_fn(void *arg)
{
	pid_t pid, ppid;
	struct sigaction sa;
	struct sigevent notif;
	char buf[5];

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();

	if (pid != CHILD_PID || ppid != PARENT_PID) {
		printf("pidns was not created\n");
		return 1;
	}

	/* Close the appropriate end of each pipe */
	close(child_to_father[0]);
	close(father_to_child[1]);

	while (read(father_to_child[0], buf, 1) != 1)
		sleep(1);

	mqd = tst_syscall(__NR_mq_open, mqname, O_RDONLY, 0, NULL);
	if (mqd == -1) {
		perror("mq_open failed");
		return 1;
	} else
		printf("mq_open succeeded\n");

	/* Register for notification on message arrival */
	notif.sigev_notify = SIGEV_SIGNAL;
	notif.sigev_signo = SIGUSR1;
	notif.sigev_value.sival_int = mqd;
	if (tst_syscall(__NR_mq_notify, mqd, &notif) == -1) {
		perror("mq_notify failed");
		return 1;
	} else
		printf("successfully registered for notification\n");

	/* Define handler for SIGUSR1 */
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = child_signal_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("sigaction failed");
		return 1;
	} else
		printf("successfully registered handler for SIGUSR1\n");

	/* Ask parent to send a message to the mqueue */
	if (write(child_to_father[1], "c:ok", 5) != 5) {
		perror("write failed");
		return 1;
	}

	sleep(3);

	/* Has parent sent a message? */
	read(father_to_child[0], buf, 5);
	if (strcmp(buf, "f:ok") != 0) {
		printf("parent did not send the message!\n");
		return 1;
	}
	printf("parent is done - cleaning up\n");

	cleanup_child();

	exit(0);
}

static void setup(void)
{
	tst_require_root();
	check_newpid();
}

int main(void)
{
	int status;
	char buf[5];
	pid_t cpid;

	setup();

	if (pipe(child_to_father) == -1 || pipe(father_to_child) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "pipe failed");
	}

	tst_syscall(__NR_mq_unlink, mqname);

	/* container creation on PID namespace */
	cpid = ltp_clone_quick(CLONE_NEWPID | SIGCHLD, child_fn, NULL);
	if (cpid == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");

	mqd =
	    tst_syscall(__NR_mq_open, mqname, O_RDWR | O_CREAT | O_EXCL, 0777,
		    NULL);
	if (mqd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "mq_open failed");
	else
		tst_resm(TINFO, "successfully created posix mqueue");

	if (write(father_to_child[1], buf, 1) != 1)
		tst_brkm(TBROK | TERRNO, cleanup, "write failed");

	/* Close the appropriate end of each pipe */
	close(child_to_father[1]);
	close(father_to_child[0]);

	/* Is container ready */
	read(child_to_father[0], buf, 5);
	if (strcmp(buf, "c:ok") != 0)
		tst_brkm(TBROK, cleanup,
			 "container did not respond as expected!");

	rc = mq_send(mqd, MSG, strlen(MSG), MSG_PRIO);
	if (rc == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "mq_send failed");
	else
		tst_resm(TINFO, "mq_send succeeded");

	/* Tell the child the message has been sent */
	if (write(father_to_child[1], "f:ok", 5) != 5)
		tst_brkm(TBROK | TERRNO, cleanup, "write failed");

	/* Wait for child to finish */
	if (wait(&status) == -1)
		tst_resm(TBROK | TERRNO, "wait failed");

	cleanup();

	tst_exit();
}
