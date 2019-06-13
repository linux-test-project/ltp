// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2013 Linux Test Project
 */
/*
 * Test /dev/kmsg based on kernel doc: Documentation/ABI/testing/dev-kmsg
 * - read() blocks
 * - non-blocking read() fails with EAGAIN
 * - partial read fails (buffer smaller than message)
 * - can write to /dev/kmsg and message seqno grows
 * - first read() after open() returns same message
 * - if messages get overwritten, read() returns -EPIPE
 * - device supports SEEK_SET, SEEK_END, SEEK_DATA
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "tst_test.h"
#include "lapi/syscalls.h"

#define MSG_PREFIX "LTP kmsg01 "
#define MAX_MSGSIZE 4096
#define NUM_READ_MSGS 3
#define NUM_READ_RETRY 10
#define NUM_OVERWRITE_MSGS 1024
#define PRINTK "/proc/sys/kernel/printk"
#define CONSOLE_LOGLEVEL_QUIET   4

static int console_loglevel = -1;

/*
 * inject_msg - write message to /dev/kmsg
 * @msg: null-terminated message to inject, should end with \n
 *
 * RETURNS:
 *   0 on success
 *  -1 on failure, errno reflects write() errno
 */
static int inject_msg(const char *msg)
{
	int f;
	f = SAFE_OPEN("/dev/kmsg", O_WRONLY);
	TEST(write(f, msg, strlen(msg)));
	SAFE_CLOSE(f);
	errno = TST_ERR;
	return TST_RET;
}

/*
 * find_msg - find message in kernel ring buffer
 * @fd:           fd to use, if < 0 function opens /dev/kmsg itself
 * @text_to_find: substring to look for in messages
 * @buf:          buf to store found message
 * @bufsize:      size of 'buf'
 * @first:        1 - return first matching message
 *                0 - return last matching message
 * RETURNS:
 *   0 on success
 *  -1 on failure
 */
static int find_msg(int fd, const char *text_to_find, char *buf, int bufsize,
	int first)
{
	int f, msg_found = 0;
	char msg[MAX_MSGSIZE + 1];

	if (fd < 0) {
		f = SAFE_OPEN("/dev/kmsg", O_RDONLY | O_NONBLOCK);
	} else {
		f = fd;
	}

	while (1) {
		TEST(read(f, msg, MAX_MSGSIZE));
		if (TST_RET < 0) {
			if (TST_ERR == EAGAIN)
				/* there are no more messages */
				break;
			else if (TST_ERR == EPIPE)
				/* current message was overwritten */
				continue;
			else
				tst_brk(TBROK|TTERRNO, "err reading /dev/kmsg");
		} else if (TST_RET < bufsize) {
			/* lines from kmsg are not NULL terminated */
			msg[TST_RET] = '\0';
			if (strstr(msg, text_to_find) != NULL) {
				strncpy(buf, msg, bufsize);
				msg_found = 1;
				if (first)
					break;
			}
		}
	}
	if (fd < 0)
		SAFE_CLOSE(f);

	if (msg_found)
		return 0;
	else
		return -1;
}

static int get_msg_fields(const char *msg, unsigned long *prio,
	unsigned long *seqno)
{
	unsigned long s, p;
	if (sscanf(msg, "%lu,%lu,", &p, &s) == 2) {
		if (prio)
			*prio = p;
		if (seqno)
			*seqno = s;
		return 0;
	} else {
		return 1;
	}
}

/*
 * timed_read - if possible reads from fd or times out
 * @fd:           fd to read from
 * @timeout_usec: timeout in useconds
 *
 * RETURNS:
 *   read bytes on successful read
 *  -1 on read() error, errno reflects read() errno
 *  -2 on timeout
 */
static int timed_read(int fd, long timeout_usec)
{
	int ret, tmp;
	struct timeval timeout;
	fd_set read_fds;

	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);
	timeout.tv_sec = timeout_usec / 1000000;
	timeout.tv_usec = timeout_usec % 1000000;

	ret = select(fd + 1, &read_fds, 0, 0, &timeout);
	switch (ret) {
	case -1:
		tst_brk(TBROK|TERRNO, "select failed");
	case 0:
		/* select timed out */
		return -2;
	}

	return read(fd, &tmp, 1);
}

/*
 * timed_read_kmsg - reads file until it reaches end of file,
 *                   read fails or times out. This ignores any
 *                   EPIPE errors.
 * @fd:           fd to read from
 * @timeout_usec: timeout in useconds for every read attempt
 *
 * RETURNS:
 *     0 on read reaching eof
 *    -1 on read error, errno reflects read() errno
 *    -2 on timeout
 */
static int timed_read_kmsg(int fd, long timeout_usec)
{
	int child, status, ret = 0;
	int pipefd[2];
	char msg[MAX_MSGSIZE];

	if (pipe(pipefd) != 0)
		tst_brk(TBROK|TERRNO, "pipe failed");

	child = fork();
	switch (child) {
	case -1:
		tst_brk(TBROK|TERRNO, "failed to fork");
	case 0:
		/* child does all the reading and keeps writing to
		 * pipe to let parent know that it didn't block */
		close(pipefd[0]);
		while (1) {
			if (write(pipefd[1], "", 1) == -1)
				tst_brk(TBROK|TERRNO, "write to pipe");
			TEST(read(fd, msg, MAX_MSGSIZE));
			if (TST_RET == 0)
				break;
			if (TST_RET == -1 && TST_ERR != EPIPE) {
				ret = TST_ERR;
				break;
			}
		}

		close(pipefd[1]);
		exit(ret);
	}
	SAFE_CLOSE(pipefd[1]);

	/* parent reads pipe until it reaches eof or until read times out */
	do {
		TEST(timed_read(pipefd[0], timeout_usec));
	} while (TST_RET > 0);
	SAFE_CLOSE(pipefd[0]);

	/* child is blocked, kill it */
	if (TST_RET == -2)
		kill(child, SIGTERM);
	SAFE_WAITPID(child, &status, 0);
	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 0) {
			return 0;
		} else {
			errno = WEXITSTATUS(status);
			return -1;
		}
	}
	return -2;
}

static void test_read_nonblock(void)
{
	int fd;

	tst_res(TINFO, "TEST: nonblock read");
	fd = SAFE_OPEN("/dev/kmsg", O_RDONLY | O_NONBLOCK);

	TEST(timed_read_kmsg(fd, 5000000));
	if (TST_RET == -1 && TST_ERR == EAGAIN)
		tst_res(TPASS, "non-block read returned EAGAIN");
	else
		tst_res(TFAIL|TTERRNO, "non-block read returned: %ld",
			TST_RET);
	SAFE_CLOSE(fd);
}

static void test_read_block(void)
{
	int fd;

	tst_res(TINFO, "TEST: blocking read");
	fd = SAFE_OPEN("/dev/kmsg", O_RDONLY);

	TEST(timed_read_kmsg(fd, 500000));
	if (TST_RET == -2)
		tst_res(TPASS, "read blocked");
	else
		tst_res(TFAIL|TTERRNO, "read returned: %ld", TST_RET);
	SAFE_CLOSE(fd);
}

static void test_partial_read(void)
{
	char msg[MAX_MSGSIZE];
	int fd;

	tst_res(TINFO, "TEST: partial read");
	fd = SAFE_OPEN("/dev/kmsg", O_RDONLY | O_NONBLOCK);

	TEST(read(fd, msg, 1));
	if (TST_RET < 0)
		tst_res(TPASS|TTERRNO, "read failed as expected");
	else
		tst_res(TFAIL, "read returned: %ld", TST_RET);
	SAFE_CLOSE(fd);
}

static void test_inject(void)
{
	char imsg[MAX_MSGSIZE], imsg_prefixed[MAX_MSGSIZE];
	char tmp[MAX_MSGSIZE];
	unsigned long prefix, seqno, seqno_last = 0;
	int i, facility, prio;

	tst_res(TINFO, "TEST: injected messages appear in /dev/kmsg");

	srand(time(NULL));
	/* test all combinations of prio 0-7, facility 0-15 */
	for (i = 0; i < 127; i++) {
		prio = (i & 7);
		facility = (i >> 3);
		sprintf(imsg, MSG_PREFIX"TEST MESSAGE %ld prio: %d, "
			"facility: %d\n", random(), prio, facility);
		sprintf(imsg_prefixed, "<%d>%s", i, imsg);

		if (inject_msg(imsg_prefixed) == -1) {
			tst_res(TFAIL|TERRNO, "inject failed");
			return;
		}

		/* check that message appears in log */
		if (find_msg(-1, imsg, tmp, sizeof(tmp), 0) == -1) {
			tst_res(TFAIL, "failed to find: %s", imsg);
			return;
		}

		/* check that facility is not 0 (LOG_KERN). */
		if (get_msg_fields(tmp, &prefix, &seqno) != 0) {
			tst_res(TFAIL, "failed to parse seqid: %s", tmp);
			return;
		}
		if (prefix >> 3 == 0) {
			tst_res(TFAIL, "facility 0 found: %s", tmp);
			return;
		}

		/* check that seq. number grows */
		if (seqno > seqno_last) {
			seqno_last = seqno;
		} else {
			tst_res(TFAIL, "seqno doesn't grow: %lu, "
				"last: %lu", seqno, seqno_last);
			return;
		}
	}

	tst_res(TPASS, "injected messages found in log");
	tst_res(TPASS, "sequence numbers grow as expected");
}

static void test_read_returns_first_message(void)
{
	unsigned long seqno[NUM_READ_MSGS + 1];
	char msg[MAX_MSGSIZE];
	int msgs_match = 1;
	int i, fd, j = NUM_READ_RETRY;

	/* Open extra fd, which we try to read after reading NUM_READ_MSGS.
	 * If this read fails with EPIPE, first message was overwritten and
	 * we should retry the whole test. If it still fails after
	 * NUM_READ_RETRY attempts, report TWARN */
	tst_res(TINFO, "TEST: mult. readers will get same first message");
	while (j) {
		fd = SAFE_OPEN("/dev/kmsg", O_RDONLY | O_NONBLOCK);

		for (i = 0; i < NUM_READ_MSGS; i++) {
			if (find_msg(-1, "", msg, sizeof(msg), 1) != 0)
				tst_res(TFAIL, "failed to find any message");
			if (get_msg_fields(msg, NULL, &seqno[i]) != 0)
				tst_res(TFAIL, "failed to parse seqid: %s",
					msg);
		}

		TEST(read(fd, msg, sizeof(msg)));
		SAFE_CLOSE(fd);
		if (TST_RET != -1)
			break;

		if (TST_ERR == EPIPE)
			tst_res(TINFO, "msg overwritten, retrying");
		else
			tst_res(TFAIL|TTERRNO, "read failed");

		/* give a moment to whoever overwrote first message to finish */
		usleep(100000);
		j--;
	}

	if (!j) {
		tst_res(TWARN, "exceeded: %d attempts", NUM_READ_RETRY);
		return;
	}

	for (i = 0; i < NUM_READ_MSGS - 1; i++)
		if (seqno[i] != seqno[i + 1])
			msgs_match = 0;
	if (msgs_match) {
		tst_res(TPASS, "all readers got same message on first read");
	} else {
		tst_res(TFAIL, "readers got different messages");
		for (i = 0; i < NUM_READ_MSGS; i++)
			tst_res(TINFO, "msg%d: %lu\n", i, seqno[i]);
	}
}

static void test_messages_overwritten(void)
{
	int i, fd;
	char msg[MAX_MSGSIZE];
	unsigned long first_seqno, seqno;
	char filler_str[] = "<7>"MSG_PREFIX"FILLER MESSAGE TO OVERWRITE OTHERS\n";

	/* Keep injecting messages until we overwrite first one.
	 * We know first message is overwritten when its seqno changes */
	tst_res(TINFO, "TEST: read returns EPIPE when messages get "
		"overwritten");
	fd = SAFE_OPEN("/dev/kmsg", O_RDONLY | O_NONBLOCK);

	if (find_msg(-1, "", msg, sizeof(msg), 1) == 0
		&& get_msg_fields(msg, NULL, &first_seqno) == 0) {
		tst_res(TINFO, "first seqno: %lu", first_seqno);
	} else {
		tst_brk(TBROK, "failed to get first seq. number");
	}

	while (1) {
		if (find_msg(-1, "", msg, sizeof(msg), 1) != 0
				|| get_msg_fields(msg, NULL, &seqno) != 0) {
			tst_res(TFAIL, "failed to get first seq. number");
			break;
		}
		if (first_seqno != seqno) {
			/* first message was overwritten */
			tst_res(TINFO, "first seqno now: %lu", seqno);
			break;
		}
		for (i = 0; i < NUM_OVERWRITE_MSGS; i++) {
			if (inject_msg(filler_str) == -1)
				tst_brk(TBROK|TERRNO, "err write to /dev/kmsg");
		}
	}

	/* first message is overwritten, so this next read should fail */
	TEST(read(fd, msg, sizeof(msg)));
	if (TST_RET == -1 && TST_ERR == EPIPE)
		tst_res(TPASS, "read failed with EPIPE as expected");
	else
		tst_res(TFAIL|TTERRNO, "read returned: %ld", TST_RET);

	/* seek position is updated to the next available record */
	tst_res(TINFO, "TEST: Subsequent reads() will return available "
		"records again");
	if (find_msg(fd, "", msg, sizeof(msg), 1) != 0)
		tst_res(TFAIL|TTERRNO, "read returned: %ld", TST_RET);
	else
		tst_res(TPASS, "after EPIPE read returned next record");

	SAFE_CLOSE(fd);
}

static int read_msg_seqno(int fd, unsigned long *seqno)
{
	char msg[MAX_MSGSIZE];

	TEST(read(fd, msg, sizeof(msg)));
	if (TST_RET == -1 && TST_ERR != EPIPE)
		tst_brk(TBROK|TTERRNO, "failed to read /dev/kmsg");

	if (TST_ERR == EPIPE)
		return -1;

	if (get_msg_fields(msg, NULL, seqno) != 0) {
		tst_res(TFAIL, "failed to parse seqid: %s", msg);
		return -1;
	}

	return 0;
}

static void test_seek(void)
{
	int j, fd, fd2;
	char msg[MAX_MSGSIZE];
	unsigned long seqno[2], tmp;
	int ret = 0;

	/* 1. read() after SEEK_SET 0 returns same (first) message */
	tst_res(TINFO, "TEST: seek SEEK_SET 0");

	for (j = 0; j < NUM_READ_RETRY && !ret; j++) {
		/*
		 * j > 0 means we are trying again, because we most likely
		 * failed on read returning EPIPE - first message in buffer
		 * has been overwrittern. Give a moment to whoever overwrote
		 * first message to finish.
		 */
		if (j)
			usleep(100000);

		/*
		 * Open 2 fds. Use fd1 to read seqno1, then seek to
		 * begininng and read seqno2. Use fd2 to check if
		 * first entry in buffer got overwritten. If so,
		 * we'll have to repeat the test.
		 */
		fd = SAFE_OPEN("/dev/kmsg", O_RDONLY | O_NONBLOCK);
		fd2 = SAFE_OPEN("/dev/kmsg", O_RDONLY | O_NONBLOCK);

		if (read_msg_seqno(fd, &seqno[0]))
			goto close_fds;

		if (lseek(fd, 0, SEEK_SET) == -1) {
			tst_res(TFAIL|TERRNO, "SEEK_SET 0 failed");
			ret = -1;
			goto close_fds;
		}

		if (read_msg_seqno(fd, &seqno[1]))
			goto close_fds;

		if (read_msg_seqno(fd2, &tmp))
			goto close_fds;

		ret = 1;
close_fds:
		SAFE_CLOSE(fd);
		SAFE_CLOSE(fd2);
	}

	if (j == NUM_READ_RETRY) {
		tst_res(TWARN, "exceeded: %d attempts", NUM_READ_RETRY);
	} else if (ret > 0) {
		if (seqno[0] != seqno[1]) {
			tst_res(TFAIL, "SEEK_SET 0, %lu != %lu",
				seqno[0], seqno[1]);
		} else {
			tst_res(TPASS, "SEEK_SET 0");
		}
	}

	/* 2. messages after SEEK_END 0 shouldn't contain MSG_PREFIX */
	fd = SAFE_OPEN("/dev/kmsg", O_RDONLY | O_NONBLOCK);
	tst_res(TINFO, "TEST: seek SEEK_END 0");
	if (lseek(fd, 0, SEEK_END) == -1)
		tst_res(TFAIL|TERRNO, "lseek SEEK_END 0 failed");
	if (find_msg(fd, MSG_PREFIX, msg, sizeof(msg), 0) != 0)
		tst_res(TPASS, "SEEK_END 0");
	else
		tst_res(TFAIL, "SEEK_END 0 found: %s", msg);

#ifdef SEEK_DATA
	/* 3. messages after SEEK_DATA 0 shouldn't contain MSG_PREFIX */
	tst_res(TINFO, "TEST: seek SEEK_DATA 0");

	/* clear ring buffer */
	if (tst_syscall(__NR_syslog, 5, NULL, 0) == -1)
		tst_brk(TBROK|TERRNO, "syslog clear failed");
	if (lseek(fd, 0, SEEK_DATA) == -1)
		tst_res(TFAIL|TERRNO, "lseek SEEK_DATA 0 failed");
	if (find_msg(fd, MSG_PREFIX, msg, sizeof(msg), 0) != 0)
		tst_res(TPASS, "SEEK_DATA 0");
	else
		tst_res(TFAIL, "SEEK_DATA 0 found: %s", msg);
#endif
	SAFE_CLOSE(fd);
}

static void test_kmsg(void)
{
	/* run test_inject first so log isn't empty for other tests */
	test_inject();
	test_read_nonblock();
	test_read_block();
	test_partial_read();
	test_read_returns_first_message();
	test_messages_overwritten();
	test_seek();
}

static void setup(void)
{
	if (access(PRINTK, F_OK) == 0) {
		SAFE_FILE_SCANF(PRINTK, "%d", &console_loglevel);
		SAFE_FILE_PRINTF(PRINTK, "%d", CONSOLE_LOGLEVEL_QUIET);
	}
}

static void cleanup(void)
{
	if (console_loglevel != -1)
		SAFE_FILE_PRINTF(PRINTK, "%d", console_loglevel);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.test_all = test_kmsg,
	.min_kver = "3.5.0"
};
