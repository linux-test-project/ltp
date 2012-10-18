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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	msgsnd06.c
 *
 * DESCRIPTION
 *	msgsnd06 - test for EIDRM error
 *
 * ALGORITHM
 *	loop if that option was specified
 *	create a message queue with read/write permissions
 *	initialize a message buffer with a known message and type
 *	enqueue messages in a loop until the queue is full
 *	fork a child process
 *	child attempts to enqueue a message to the full queue and sleeps
 *	parent removes the queue and then exits
 *	child get a return from msgsnd()
 *	check the errno value
 *	  issue a PASS message if we get EIDRM
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  msgsnd06 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *      14/03/2008 Matthieu Fertré (Matthieu.Fertre@irisa.fr)
 *      - Fix concurrency issue. Due to the use of usleep function to
 *        synchronize processes, synchronization issues can occur on a loaded
 *        system. Fix this by using pipes to synchronize processes.
 *
 * RESTRICTIONS
 *	none
 */

#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

#include "ipcmsg.h"
#include "libtestsuite.h"

void cleanup(void);
void setup(void);
void do_child(void);

char *TCID = "msgsnd06";
int TST_TOTAL = 1;

int exp_enos[] = { EIDRM, 0 };	/* 0 terminated list of expected errnos */

int msg_q_1 = -1;		/* The message queue id created in setup */

int sync_pipes[2];

MSGBUF msg_buf;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t c_pid;
	int status, e_code;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}
#ifdef UCLINUX
#define PIPE_NAME	"msgsnd06"
	maybe_run_child(&do_child, "d", &msg_q_1);
#endif

	setup();		/* global setup */

	if (sync_pipe_create(sync_pipes, PIPE_NAME) == -1)
		tst_brkm(TBROK, cleanup, "sync_pipe_create failed");

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		msgkey = getipckey();

		/* create a message queue with read/write permission */
		if ((msg_q_1 = msgget(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW))
		    == -1) {
			tst_brkm(TBROK, cleanup, "Can't create message queue");
		}

		/* initialize the message buffer */
		init_buf(&msg_buf, MSGTYPE, MSGSIZE);

		/* write messages to the queue until it is full */
		while (msgsnd(msg_q_1, &msg_buf, MSGSIZE, IPC_NOWAIT) != -1) {
			msg_buf.mtype += 1;
		}

		/*
		 * fork a child that will attempt to write a message
		 * to the queue without IPC_NOWAIT
		 */
		if ((c_pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "could not fork");
		}

		if (c_pid == 0) {	/* child */

#ifdef UCLINUX
			if (self_exec(av[0], "d", msg_q_1) < 0) {
				tst_brkm(TBROK, cleanup, "could not self_exec");
			}
#else
			do_child();
#endif
		} else {	/* parent */

			if (sync_pipe_wait(sync_pipes) == -1)
				tst_brkm(TBROK, cleanup,
					 "sync_pipe_wait failed");

			if (sync_pipe_close(sync_pipes, PIPE_NAME) == -1)
				tst_brkm(TBROK, cleanup,
					 "sync_pipe_close failed");

			/* After son has been created, give it a chance to execute the
			 * msgsnd command before we continue. Without this sleep, on SMP machine
			 * the father rm_queue could be executed before the son msgsnd.
			 */
			sleep(2);
			/* remove the queue */
			rm_queue(msg_q_1);

			/* wait for the child to finish */
			wait(&status);
			/* make sure the child returned a good exit status */
			e_code = status >> 8;
			if (e_code != 0) {
				tst_resm(TFAIL, "Failures reported above");
			}
		}
	}

	cleanup();

	tst_exit();
}

/*
 * do_child()
 */
void do_child()
{
	int retval = 0;

#ifdef UCLINUX
	/* initialize the message buffer */
	init_buf(&msg_buf, MSGTYPE, MSGSIZE);

	if (sync_pipe_create(sync_pipes, PIPE_NAME) == -1)
		tst_brkm(TBROK, cleanup, "sync_pipe_create failed");
#endif

	if (sync_pipe_notify(sync_pipes) == -1)
		tst_brkm(TBROK, cleanup, "sync_pipe_notify failed");

	if (sync_pipe_close(sync_pipes, PIPE_NAME) == -1)
		tst_brkm(TBROK, cleanup, "sync_pipe_close failed");
	/*
	 * Attempt to write another message to the full queue.
	 * Without the IPC_NOWAIT flag, the child sleeps
	 */
	TEST(msgsnd(msg_q_1, &msg_buf, MSGSIZE, 0));

	if (TEST_RETURN != -1) {
		retval = 1;
		tst_resm(TFAIL, "call succeeded when error expected");
		exit(retval);
	}

	TEST_ERROR_LOG(TEST_ERRNO);

	switch (TEST_ERRNO) {
	case EIDRM:
		tst_resm(TPASS, "expected failure - errno = %d : %s",
			 TEST_ERRNO, strerror(TEST_ERRNO));

		/* mark the queue as invalid as it was removed */
		msg_q_1 = -1;
		break;
	default:
		retval = 1;
		tst_resm(TFAIL,
			 "call failed with an unexpected error - %d : %s",
			 TEST_ERRNO, strerror(TEST_ERRNO));
		break;
	}
	exit(retval);
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{

	tst_rmdir();

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}
