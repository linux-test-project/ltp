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
 *	msgctl04.c
 *
 * DESCRIPTION
 *	msgctl04 - test for EACCES, EFAULT and EINVAL errors using
 *		   a variety of incorrect calls.
 *
 * ALGORITHM
 *	create two message queues
 *	loop if that option was specified
 *	try to access a queue with some invalid argument
 *	check the errno value
 *	  issue a PASS message if we get EACCES, EFAULT or EINVAL
 *	  depending on the test case
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  msgctl04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *      12/03/2008 Matthieu Fertré (Matthieu.Fertre@irisa.fr)
 *      - Fix concurrency issue. The second key used for this test could
 *        conflict with the key from another task.
 *
 * RESTRICTIONS
 *	none
 */
#include <pwd.h>

#include "test.h"

#include "ipcmsg.h"

char *TCID = "msgctl04";
int TST_TOTAL = 6;

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int msg_q_1 = -1;		/* The message queue id created in setup */
int msg_q_2 = -1;		/* Another queue id created in setup */
int bad_q = -1;			/* a value to use as a bad queue id */

struct msqid_ds q_buf;

struct test_case_t {		/* This allows testing of many negative */
	int *queue_id;		/* test cases that can all use the same */
	int ipc_cmd;		/* basic test setup.                    */
	struct msqid_ds *buf;
	int error;
} TC[] = {
	/* EACCES - there is no read permission for the queue */
	{
	&msg_q_1, IPC_STAT, &q_buf, EACCES},
	    /* EFAULT - the structure address is invalid - IPC_STAT */
	{
	&msg_q_2, IPC_STAT, (struct msqid_ds *)-1, EFAULT},
	    /* EFAULT - the structure address is invalid - IPC_SET */
	{
	&msg_q_2, IPC_SET, (struct msqid_ds *)-1, EFAULT},
	    /* EINVAL - the command (-1) is invalid */
	{
	&msg_q_2, -1, &q_buf, EINVAL},
	    /* EINVAL - the queue id is invalid - IPC_STAT */
	{
	&bad_q, IPC_STAT, &q_buf, EINVAL},
	    /* EINVAL - the queue id is invalid - IPC_SET */
	{
	&bad_q, IPC_SET, &q_buf, EINVAL}
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

		/* loop through the test cases */

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(msgctl(*(TC[i].queue_id), TC[i].ipc_cmd,
				    TC[i].buf));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "msgctl() call succeeded "
					 "on expected fail");
				continue;
			}

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS | TTERRNO, "expected failure");
			} else {
				tst_resm(TFAIL | TTERRNO, "unexpected error");
				tst_resm(TINFO, "expected error is - %d : %s",
					 TC[i].error, strerror(TC[i].error));
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
	if (setuid(ltpuser->pw_uid) == -1)
		tst_resm(TINFO, "setuid(%d) failed", ltpuser->pw_uid);

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	msgkey = getipckey();

	/* Get an new IPC resource key. */
	msgkey2 = getipckey();

	/* now we have a key, so let's create a message queue */
	if ((msg_q_1 = msgget(msgkey, IPC_CREAT | IPC_EXCL)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Can't create message queue #1");
	}

	/* now let's create another message queue with read & write access */
	if ((msg_q_2 =
	     msgget(msgkey2, IPC_CREAT | IPC_EXCL | MSG_RD | MSG_WR)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Can't create message queue #2");
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * remove the message queues that were created.
	 */
	rm_queue(msg_q_1);

	rm_queue(msg_q_2);

	tst_rmdir();

}
