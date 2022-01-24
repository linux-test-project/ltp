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
* File: pidns31.c
*
*   Description:
*    This testcase checks if the si_pid is correctly set when a process
*    that has registered for notification on a posix mqueue is in an
*    ancestor namespace wrt the process that sends a message to that posix
*    mqueue.
*
*   Test Assertion & Strategy:
*    Parent                                   Child
*    --------------------------------------------------------------------------
*    Create a POSIX mqueue.
*    Create a PID namespace container.
*    Register for notification when a
*       message arrives in that mqueue
*    Install a handler for SIGUSR1.
*                                             Open that mqueue for writing
*                                             Write something to the mqueue.
*    Inside the handler, check that
*       si_pid is set to the child's pid
*
*   Usage: <for command-line>
*    pidns31
*
*   History:
*    DATE      NAME                             DESCRIPTION
*    04/12/08  Nadia Derbey               Creation of this test.
*              <Nadia.Derbey@bull.net>
*
******************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
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

char *TCID = "pidns31";
int TST_TOTAL = 1;

char *mqname = "mq1";
int result = TFAIL;

int father_to_child[2];

#define CHILD_PID       1
#define PARENT_PID      0

#define MSG      "HOW ARE YOU"
#define MSG_PRIO 1

#define NO_STEP -1
#define F_STEP_0 0x00
#define F_STEP_1 0x01
#define F_STEP_2 0x02
#define F_STEP_3 0x03
#define C_STEP_0 0x10
#define C_STEP_1 0x11

struct notify_info {
	mqd_t mqd;
	pid_t pid;
};

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

/*
 * steps F_STEP_XX : called from main
 * steps C_STEP_XX : called from child_fn
 */
static void cleanup_resources(int step, mqd_t mqd)
{
	switch (step) {
	case C_STEP_1:
		close(father_to_child[0]);
		/* fall through */
	case C_STEP_0:
		mq_close(mqd);
		break;

	case F_STEP_3:
		remove_mqueue(mqd);
		close(father_to_child[1]);
		break;

	case F_STEP_2:
		tst_syscall(__NR_mq_notify, mqd, NULL);
		/* fall through */
	case F_STEP_1:
		remove_mqueue(mqd);
		/* fall through */
	case F_STEP_0:
		remove_pipe(father_to_child);
		break;
	default:
		tst_resm(TWARN, "Unknown code - no resource removed.");
		break;
	}
}

/*
 * cleanup_mqueue() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 * step == -1 means no local resource to remove.
 */
void cleanup_mqueue(int result, int step, mqd_t mqd)
{
	if (step != NO_STEP)
		cleanup_resources(step, mqd);

	tst_exit();
}

/*
 * child_fn() - Inside container
 */
int child_fn(void *arg)
{
	pid_t pid, ppid;
	mqd_t mqd;
	char buf[5];

	/* Set process id and parent pid */
	pid = getpid();
	ppid = getppid();

	if (pid != CHILD_PID || ppid != PARENT_PID) {
		tst_resm(TBROK, "cinit: pidns is not created");
		cleanup_mqueue(TBROK, NO_STEP, 0);
	}

	/* Close the appropriate end of pipe */
	close(father_to_child[1]);

	/* Is parent ready to receive a message? */
	read(father_to_child[0], buf, 5);
	if (strcmp(buf, "f:ok")) {
		tst_resm(TBROK, "cinit: parent did not send the message!");
		cleanup_mqueue(TBROK, NO_STEP, 0);
	}
	tst_resm(TINFO, "cinit: my father is ready to receive a message");

	mqd = tst_syscall(__NR_mq_open, mqname, O_WRONLY, 0, NULL);
	if (mqd == (mqd_t) - 1) {
		tst_resm(TBROK, "cinit: mq_open() failed (%s)",
			 strerror(errno));
		cleanup_mqueue(TBROK, NO_STEP, 0);
	}
	tst_resm(TINFO, "cinit: mq_open succeeded");

	if (mq_send(mqd, MSG, strlen(MSG), MSG_PRIO) == (mqd_t) - 1) {
		tst_resm(TBROK, "cinit: mq_send() failed (%s)",
			 strerror(errno));
		cleanup_mqueue(TBROK, C_STEP_0, mqd);
	}
	tst_resm(TINFO, "cinit: mq_send() succeeded");

	/* Cleanup and exit */
	cleanup_resources(C_STEP_1, mqd);
	exit(0);
}

/*
 * father_signal_handler()
 */
static void father_signal_handler(int sig, siginfo_t * si, void *unused)
{
	char buf[256];
	struct mq_attr attr;
	struct notify_info *info;

	if (si->si_signo != SIGUSR1) {
		tst_resm(TBROK, "father: received %s unexpectedly",
			 strsignal(si->si_signo));
		return;
	}

	if (si->si_code != SI_MESGQ) {
		tst_resm(TBROK, "father: expected signal code SI_MESGQ - "
			 "Got %d", si->si_code);
		return;
	}

	if (!si->si_ptr) {
		tst_resm(TBROK, "father: expected si_ptr - Got NULL");
		return;
	}

	info = (struct notify_info *)si->si_ptr;

	if (si->si_pid != info->pid) {
		tst_resm(TFAIL,
			 "father: expected signal originator PID = %d - Got %d",
			 info->pid, si->si_pid);
		return;
	}

	tst_resm(TPASS, "father: signal originator PID = %d", si->si_pid);
	result = TPASS;

	/*
	 * Now read the message - Be silent on errors since this is not the
	 * test purpose.
	 */
	if (!mq_getattr(info->mqd, &attr))
		mq_receive(info->mqd, buf, attr.mq_msgsize, NULL);
}

static void setup(void)
{
	tst_require_root();
	check_newpid();
}

/***********************************************************************
*   M A I N
***********************************************************************/

int main(void)
{
	pid_t cpid;
	mqd_t mqd;
	struct sigevent notif;
	struct sigaction sa;
	int status;
	struct notify_info info;

	setup();

	if (pipe(father_to_child) == -1) {
		tst_resm(TBROK, "parent: pipe() failed. aborting!");
		cleanup_mqueue(TBROK, NO_STEP, 0);
	}

	tst_syscall(__NR_mq_unlink, mqname);
	mqd =
	    tst_syscall(__NR_mq_open, mqname, O_RDWR | O_CREAT | O_EXCL, 0777,
		    NULL);
	if (mqd == (mqd_t) - 1) {
		tst_resm(TBROK, "parent: mq_open() failed (%s)",
			 strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_0, 0);
	}
	tst_resm(TINFO, "parent: successfully created posix mqueue");

	/* container creation on PID namespace */
	cpid = ltp_clone_quick(CLONE_NEWPID | SIGCHLD, child_fn, NULL);
	if (cpid < 0) {
		tst_resm(TBROK, "parent: clone() failed(%s)", strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_1, mqd);
	}
	tst_resm(TINFO, "parent: successfully created child (pid = %d)", cpid);

	/* Register for notification on message arrival */
	notif.sigev_notify = SIGEV_SIGNAL;
	notif.sigev_signo = SIGUSR1;
	info.mqd = mqd;
	info.pid = cpid;
	notif.sigev_value.sival_ptr = &info;
	if (tst_syscall(__NR_mq_notify, mqd, &notif) == (mqd_t) -1) {
		tst_resm(TBROK, "parent: mq_notify() failed (%s)",
			 strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_1, mqd);
	}
	tst_resm(TINFO, "parent: successfully registered for notification");

	/* Define handler for SIGUSR1 */
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = father_signal_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		tst_resm(TBROK, "parent: sigaction() failed(%s)",
			 strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_2, mqd);
	}
	tst_resm(TINFO, "parent: successfully registered handler for SIGUSR1");

	/* Close the appropriate end of pipe */
	close(father_to_child[0]);

	/* Tell the child a message can be sent */
	if (write(father_to_child[1], "f:ok", 5) != 5) {
		tst_resm(TBROK, "parent: pipe is broken(%s)", strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_2, mqd);
	}

	sleep(3);

	/* Wait for child to finish */
	if (wait(&status) == -1) {
		tst_resm(TBROK, "parent: wait() failed(%s)", strerror(errno));
		cleanup_mqueue(TBROK, F_STEP_1, mqd);
	}

	cleanup_mqueue(result, F_STEP_3, mqd);

	tst_exit();
}
