/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Description:
 *     Basic test for msgrcv(2) using MSG_EXCEPT, MSG_NOERROR
 */

#define  _GNU_SOURCE
#include <sys/wait.h>
#include "test.h"
#include "ipcmsg.h"


#define MSGTYPE1	1
#define MSGTYPE2	2
#define MSG1	"message type1"
#define MSG2	"message type2"

static void wait4child(pid_t child, char *tst_flag);

static void test_msg_except(void);
static void test_msg_noerror(void);

static void (*testfunc[])(void) = { test_msg_except, test_msg_noerror };

char *TCID = "msgrcv07";
int TST_TOTAL = ARRAY_SIZE(testfunc);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

static void test_msg_except(void)
{
	pid_t child_pid;
	int msgq_id;
	MSGBUF snd_buf1 = {.mtype = MSGTYPE1, .mtext = MSG1};
	MSGBUF snd_buf2 = {.mtype = MSGTYPE2, .mtext = MSG2};
	MSGBUF rcv_buf;

	msgq_id = msgget(IPC_PRIVATE, MSG_RW);
	if (msgq_id == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Can't create message queue");

	if (msgsnd(msgq_id, &snd_buf1, MSGSIZE, 0) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Can't enqueue message");

	if (msgsnd(msgq_id, &snd_buf2, MSGSIZE, 0) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Can't enqueue message");

	child_pid = tst_fork();
	if (child_pid == -1) {
		tst_brkm(TBROK, cleanup, "fork failed");
	} else if (child_pid > 0) {
		wait4child(child_pid, "MSG_EXCEPT");
	} else {
		memset(&rcv_buf, 0, sizeof(rcv_buf));
		TEST(msgrcv(msgq_id, &rcv_buf, MSGSIZE, MSGTYPE2, MSG_EXCEPT));
		if (TEST_RETURN == -1) {
			fprintf(stderr, "msgrcv(MSG_EXCEPT) failed\n");
			exit(TBROK);
		}
		/* check the received message */
		if (strcmp(rcv_buf.mtext, MSG1) == 0 &&
		    rcv_buf.mtype == MSGTYPE1)
			exit(TPASS);
		else
			exit(TFAIL);
	}

	rm_queue(msgq_id);
}


static void test_msg_noerror(void)
{
	pid_t child_pid;
	int msg_len, msgq_id;
	MSGBUF snd_buf1 = {.mtype = MSGTYPE1, .mtext = MSG1};
	MSGBUF rcv_buf;

	msgq_id = msgget(IPC_PRIVATE, MSG_RW);
	if (msgq_id == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Can't create message queue");

	if (msgsnd(msgq_id, &snd_buf1, MSGSIZE, 0) == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Can't enqueue message");

	child_pid = tst_fork();
	if (child_pid == -1) {
		tst_brkm(TBROK, cleanup, "fork failed");
	} else if (child_pid > 0) {
		wait4child(child_pid, "MSG_NOERROR");
	} else {
		msg_len = sizeof(MSG1) / 2;
		memset(&rcv_buf, 0, sizeof(rcv_buf));

		TEST(msgrcv(msgq_id, &rcv_buf, msg_len, MSGTYPE1, MSG_NOERROR));
		if (TEST_RETURN == -1)
			exit(TFAIL);

		if (strncmp(rcv_buf.mtext, MSG1, msg_len) == 0 &&
		    rcv_buf.mtype == MSGTYPE1)
			exit(TPASS);
		exit(TFAIL);
	}

	rm_queue(msgq_id);
}

static void wait4child(pid_t child, char *tst_flag)
{
	int status;
	int ret;

	if (waitpid(child, &status, 0) == -1)
		tst_resm(TBROK | TERRNO, "waitpid");
	if (WIFEXITED(status)) {
		ret = WEXITSTATUS(status);
		if (ret == 0)
			tst_resm(TPASS, "test %s success", tst_flag);
		else if (ret == 1)
			tst_resm(TFAIL, "test %s failed", tst_flag);
		else
			tst_brkm(TBROK, cleanup, "msgrcv failed unexpectedly");
	} else {
		tst_brkm(TBROK, cleanup, "child process terminated "
			 "abnormally. status: %d", status);
	}
}

void cleanup(void)
{
}
