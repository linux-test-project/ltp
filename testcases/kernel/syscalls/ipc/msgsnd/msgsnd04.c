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
 *	msgsnd04.c
 *
 * DESCRIPTION
 *	msgsnd04 - test for EAGAIN error
 *
 * ALGORITHM
 *	create a message queue with read/write permissions
 *	initialize a message buffer with a known message and type
 *	enqueue the message in a loop until the queue is full
 *	loop if that option was specified
 *	attempt to enqueue another message - msgsnd()
 *	check the errno value
 *	  issue a PASS message if we get EAGAIN
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  msgsnd04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "test.h"
#include "usctest.h"

#include "ipcmsg.h"

void cleanup(void);
void setup(void);

char *TCID = "msgsnd04";
int TST_TOTAL = 1;

int exp_enos[] = { EAGAIN, 0 };	/* 0 terminated list of expected errnos */

int msg_q_1 = -1;		/* The message queue id created in setup */
MSGBUF msg_buf;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * Attempt to write another message to the full queue.
		 */

		TEST(msgsnd(msg_q_1, &msg_buf, MSGSIZE, IPC_NOWAIT));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded when error expected");
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		switch (TEST_ERRNO) {
		case EAGAIN:
			tst_resm(TPASS, "expected failure - errno = %d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "call failed with an "
				 "unexpected error - %d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		}
	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	msgkey = getipckey();

	/* create a message queue with read/write permission */
	if ((msg_q_1 = msgget(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW)) == -1) {
		tst_brkm(TBROK, cleanup, "Can't create message queue");
	}

	/* initialize the message buffer */
	init_buf(&msg_buf, MSGTYPE, MSGSIZE);

	/* write messages to the queue until it is full */
	while (msgsnd(msg_q_1, &msg_buf, MSGSIZE, IPC_NOWAIT) != -1) {
		msg_buf.mtype += 1;
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/* if it exists, remove the message queue that was created */
	rm_queue(msg_q_1);

	tst_rmdir();

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}
