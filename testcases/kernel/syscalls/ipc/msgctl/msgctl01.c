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
 *	msgctl01.c
 *
 * DESCRIPTION
 *	msgctl01 - create a message queue, then issue the IPC_STAT command
 *		   and RMID commands to test the functionality
 *
 * ALGORITHM
 *	create a message queue
 *	loop if that option was specified
 *	call msgctl() with the IPC_STAT command
 *	check the return code
 *	  if failure, issue a FAIL message and break remaining tests
 *	otherwise,
 *	  if doing functionality testing
 *	  	if the max number of bytes on the queue is > 0,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	  else issue a PASS message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  msgctl01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
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

#include "ipcmsg.h"

char *TCID = "msgctl01";
int TST_TOTAL = 1;

int msg_q_1 = -1;		/* to hold the message queue id */

struct msqid_ds qs_buf;

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/*
		 * Get the msqid_ds structure values for the queue
		 */

		TEST(msgctl(msg_q_1, IPC_STAT, &qs_buf));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "msgctl() call failed");
		} else {
			if (qs_buf.msg_qbytes > 0) {
				tst_resm(TPASS, "qs_buf.msg_qbytes is"
					 " a positive value");
			} else {
				tst_resm(TFAIL, "qs_buf.msg_qbytes did"
					 " not change");
			}
		}

		/*
		 * clean up things in case we are looping
		 */
		qs_buf.msg_qbytes = 0x0000;
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

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	/* get a message key */
	msgkey = getipckey();

	/* make sure the initial # of bytes is 0 in our buffer */
	qs_buf.msg_qbytes = 0x0000;

	/* now we have a key, so let's create a message queue */
	if ((msg_q_1 = msgget(msgkey, IPC_CREAT | IPC_EXCL | MSG_RW)) == -1) {
		tst_brkm(TBROK, cleanup, "Can't create message queue");
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/* if it exists, remove the message queue */
	rm_queue(msg_q_1);

	tst_rmdir();

}
