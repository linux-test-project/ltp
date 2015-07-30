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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	msgrcv02.c
 *
 * DESCRIPTION
 *	msgrcv02 - test for EACCES and EFAULT errors
 *
 * ALGORITHM
 *	create a message queue with read/write permissions
 *	initialize a message buffer with a known message and type
 *	enqueue the message
 *	create another message queue without read/write permissions
 *	loop if that option was specified
 *	call msgrcv() using two different invalid cases
 *	check the errno value
 *	  issue a PASS message if we get EACCES or EFAULT
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  msgrcv02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *      - Fix concurrency issue. The second key used for this test could
 *        conflict with the key from another task.

 *
 * RESTRICTIONS
 *	none
 */

#include <pwd.h>

#include "test.h"

#include "ipcmsg.h"

void cleanup(void);
void setup(void);

char *TCID = "msgrcv02";
int TST_TOTAL = 2;

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int msg_q_1 = -1;		/* The message queue ID created in setup */
int msg_q_2 = -1;		/* Another message queue ID created in setup */
MSGBUF snd_buf, rcv_buf;

struct test_case_t {
	int *queue_id;
	MSGBUF *mbuf;
	int error;
} TC[] = {
	/* EACCES - the queue has no read access */
	{
	&msg_q_2, &rcv_buf, EACCES},
	    /* EFAULT - the message buffer address is invalid */
	{
	&msg_q_1, (MSGBUF *) - 1, EFAULT}
};

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			/*
			 * Use the TEST macro to make the call
			 */

			TEST(msgrcv(*(TC[i].queue_id), TC[i].mbuf, MSGSIZE,
				    1, IPC_NOWAIT));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - errno = "
					 "%d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "call failed with an "
					 "unexpected error - %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			}
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
	key_t msgkey2;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Switch to nobody user for correct error code collection */
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	msgkey = getipckey();

	/* Get an new IPC resource key. */
	msgkey2 = getipckey();

	/* create a message queue with read/write permission */
	if ((msg_q_1 = msgget(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW)) == -1) {
		tst_brkm(TBROK, cleanup, "Can't create message queue #1");
	}

	/* initialize a message buffer */
	init_buf(&snd_buf, MSGTYPE, MSGSIZE);

	/* put it on msq_q_1 */
	if (msgsnd(msg_q_1, &snd_buf, MSGSIZE, IPC_NOWAIT) == -1) {
		tst_brkm(TBROK, cleanup, "Couldn't put message on queue");
	}

	/* create a message queue without read/write permission */
	if ((msg_q_2 = msgget(msgkey2, IPC_CREAT | IPC_EXCL)) == -1) {
		tst_brkm(TBROK, cleanup, "Can't create message queue #2");
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/* if it exists, remove the message queue #1 */
	rm_queue(msg_q_1);

	/* if it exists, remove the message queue #2 */
	rm_queue(msg_q_2);

	tst_rmdir();

}
