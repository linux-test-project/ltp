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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
#include "usctest.h"
#include "test.h"
#include "linux_syscall_numbers.h"
#include "libclone.h"

char *TCID = "pidns30";
int TST_TOTAL = 1;

char *mqname = "mq1";
int result = TFAIL;

int errno;
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

static void remove_pipe(int *fd)
{
	close(fd[0]);
	close(fd[1]);
}

static void remove_mqueue(mqd_t mqd)
{
	mq_close(mqd);
	syscall(__NR_mq_unlink, mqname);
}

/*
 * steps F_STEP_XX : called from main
 * steps C_STEP_XX : called from child_fn
 */
static void cleanup_resources(int step, mqd_t mqd)
{
	switch (step) {
	case C_STEP_2:
		close(child_to_father[1]);
		close(father_to_child[0]);
		mq_close(mqd);
		break;

	case C_STEP_1:
		syscall(__NR_mq_notify, mqd, NULL);
		/* fall through */
	case C_STEP_0:
		mq_close(mqd);
		break;

	case F_STEP_3:
		remove_mqueue(mqd);
		close(child_to_father[0]);
		close(father_to_child[1]);
		break;

	case F_STEP_2:
		remove_mqueue(mqd);
		/* fall through */
	case F_STEP_1:
		remove_pipe(father_to_child);
		/* fall through */
	case F_STEP_0:
		remove_pipe(child_to_father);
		break;
	default:
		tst_resm(TWARN, "Unknown code - no resource removed.");
		break;
	}
}

/*
 * cleanup_mqueue() - performs all ONE TIME cleanup for this test at
 *             	      completion or premature exit.
 */
static void cleanup_mqueue(int result, int step, mqd_t mqd)
{
	if (step != NO_STEP)
		cleanup_resources(step, mqd);

	/* Clean the test testcase as LTP wants*/
	TEST_CLEANUP;

	tst_exit();
}

/*
 * child_signal_handler() - to handle SIGUSR1
 */
static void child_signal_handler(int sig, siginfo_t *si, void *unused)
{
	char buf[256];
	mqd_t rc;
	struct mq_attr attr;

	if (si->si_signo != SIGUSR1) {
		tst_resm(TBROK, "cinit: received %s unexpectedly",
			strsignal(si->si_signo));
		return;
	}

	if (si->si_code != SI_MESGQ) {
		tst_resm(TBROK, "cinit: expected signal code SI_MESGQ - Got %d",
			si->si_code);
		return;
	}

	if (si->si_pid) {
		tst_resm(TFAIL,
			"cinit: expected signal originator PID = 0 - Got %d",
			si->si_pid);
		return;
	}

	tst_resm(TPASS, "cinit: signal originator PID = 0");
	result = TPASS;

	/*
	 * Now read the message - Be silent on errors since this is not the
	 * test purpose.
	 */
	rc = mq_getattr((mqd_t)si->si_int, &attr);
	if (rc == (mqd_t)-1)
		return;

	mq_receive((mqd_t)si->si_int, buf, attr.mq_msgsize, NULL);
}


/*
 * child_fn() - Inside container
 */
int child_fn(void *arg)
{
	pid_t pid, ppid;
	struct sigaction sa;
	mqd_t mqd;
	struct sigevent notif;
	char buf[5];

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();

	if (pid != CHILD_PID || ppid != PARENT_PID) {
		tst_resm(TBROK, "cinit: pidns is not created");
		cleanup_mqueue(TBROK, NO_STEP, 0);
	}

	/* Close the appropriate end of each pipe */
	close(child_to_father[0]);
	close(father_to_child[1]);

	mqd = syscall(__NR_mq_open, mqname, O_RDONLY);
	if (mqd == (mqd_t)-1) {
		tst_resm(TBROK, "cinit: mq_open() failed (%s)",
			strerror(errno));
		cleanup_mqueue(TBROK, NO_STEP, 0);
	}
	tst_resm(TINFO, "cinit: mq_open succeeded");

	/* Register for notification on message arrival */
	notif.sigev_notify = SIGEV_SIGNAL;
	notif.sigev_signo = SIGUSR1;
	notif.sigev_value.sival_int = mqd;
	if (syscall(__NR_mq_notify, mqd, &notif) == (mqd_t)-1) {
		tst_resm(TBROK, "cinit: mq_notify() failed (%s)",
			strerror(errno));
		cleanup_mqueue(TBROK, C_STEP_0, mqd);
	}
	tst_resm(TINFO, "cinit: successfully registered for notification");

	/* Define handler for SIGUSR1 */
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = child_signal_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		tst_resm(TBROK, "cinit: sigaction() failed(%s)",
			strerror(errno));
		cleanup_mqueue(TBROK, C_STEP_1, mqd);
	}
	tst_resm(TINFO, "cinit: successfully registered handler for SIGUSR1");

	/* Ask parent to send a message to the mqueue */
	if (write(child_to_father[1], "c:ok", 5) != 5) {
		tst_resm(TBROK, "cinit: pipe is broken (%s)", strerror(errno));
		cleanup_mqueue(TBROK, C_STEP_1, mqd);
	}

	sleep(3);

	/* Has parent sent a message? */
	read(father_to_child[0], buf, 5);
	if (strcmp(buf, "f:ok")) {
		tst_resm(TBROK, "cinit: parent did not send the message!");
		cleanup_mqueue(TBROK, C_STEP_1, mqd);
	}
	tst_resm(TINFO, "cinit: my father is done - cleaning");

	/* Be silent on errors from now on */
	cleanup_resources(C_STEP_2, mqd);

	exit(0);
}

/***********************************************************************
*   M A I N
***********************************************************************/

int main(int argc, char *argv[])
{
	int status;
	char buf[5];
	pid_t cpid;
	mqd_t mqd;
	mqd_t rc;

	if (pipe(child_to_father) == -1) {
		tst_resm(TBROK, "parent: pipe() failed. aborting!");
		cleanup_mqueue(TBROK, NO_STEP, 0);
	}

	if (pipe(father_to_child) == -1) {
		tst_resm(TBROK, "parent: pipe() failed. aborting!");
		cleanup_mqueue(TBROK, F_STEP_0, 0);
	}

	syscall(__NR_mq_unlink, mqname);
	mqd = syscall(__NR_mq_open, mqname, O_RDWR|O_CREAT|O_EXCL, 0777, NULL);
	if (mqd == (mqd_t)-1) {
		tst_resm(TBROK, "parent: mq_open() failed (%s)",
			strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_1, 0);
	}
	tst_resm(TINFO, "parent: successfully created posix mqueue");

	/* container creation on PID namespace */
	cpid = ltp_clone_quick(CLONE_NEWPID|SIGCHLD, child_fn, NULL);
	if (cpid < 0) {
		tst_resm(TBROK, "parent: clone() failed(%s)", strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_2, mqd);
	}

	/* Close the appropriate end of each pipe */
	close(child_to_father[1]);
	close(father_to_child[0]);

	/* Is container ready */
	read(child_to_father[0], buf, 5);
	if (strcmp(buf, "c:ok")) {
		tst_resm(TBROK, "parent: container did not respond!");
		cleanup_mqueue(TBROK, F_STEP_2, mqd);
	}

	rc = mq_send(mqd, MSG, strlen(MSG), MSG_PRIO);
	if (rc == (mqd_t)-1) {
		tst_resm(TBROK, "parent: mq_send() failed (%s)",
			strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_2, mqd);
	}
	tst_resm(TINFO, "parent: mq_send() succeeded");

	/* Tell the child the message has been sent */
	if (write(father_to_child[1], "f:ok", 5) != 5) {
		tst_resm(TBROK, "father: pipe is broken(%s)", strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_2, mqd);
	}

	/* Wait for child to finish */
	if (wait(&status) == -1) {
		tst_resm(TBROK, "parent: wait() failed(%s)", strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_2, mqd);
	}

	cleanup_mqueue(result, F_STEP_3, mqd);

	/* NOT REACHED */
	return 0;
}    /* End main */
