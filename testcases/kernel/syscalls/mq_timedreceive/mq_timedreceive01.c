/*
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd
 *          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * Copyright (c) 2016-2017 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <errno.h>
#include <limits.h>
#include <mqueue.h>

#include "tst_test.h"
#include "tst_sig_proc.h"
#include "tst_safe_posix_ipc.h"

static struct sigaction act;
static pid_t pid;
static int fd, fd_root;
static struct timespec timeout_ts;
static struct timespec eintr_ts;

struct test_case {
	int len;
	unsigned prio;
	struct timespec *rq;
	int fd;
	int invalid_msg;
	int send;
	int ret;
	int err;
	void (*setup)(void);
	void (*cleanup)(void);
};

#define MAX_MSGSIZE     8192

#define QUEUE_NAME	"/test_mqueue"

static void create_queue(void);
static void create_queue_nonblock(void);
static void create_queue_sig(void);
static void create_queue_timeout(void);
static void open_fd(void);
static void unlink_queue(void);
static void unlink_queue_sig(void);

static const struct test_case tcase[] = {
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.send = 1,
		.len = 0,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.send = 1,
		.len = 1,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.send = 1,
		.len = MAX_MSGSIZE,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.send = 1,
		.len = 1,
		.prio = 32767,	/* max priority */
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.invalid_msg = 1,
		.send = 1,
		.len = 0,
		.ret = -1,
		.err = EMSGSIZE,
	},
	{
		.len = 0,
		.fd = -1,
		.ret = -1,
		.err = EBADF,
	},
	{
		.len = 0,
		.fd = INT_MAX - 1,
		.ret = -1,
		.err = EBADF,
	},
	{
		.len = 0,
		.ret = -1,
		.err = EBADF,
		.setup = open_fd,
	},
	{
		.len = 16,
		.ret = -1,
		.err = EAGAIN,
		.setup = create_queue_nonblock,
		.cleanup = unlink_queue,
	},
	{
		.len = 16,
		.rq = &(struct timespec) {.tv_sec = -1, .tv_nsec = 0},
		.ret = -1,
		.err = EINVAL,
		.setup = create_queue,
		.cleanup = unlink_queue,
	},
	{
		.len = 16,
		.rq = &(struct timespec) {.tv_sec = 0, .tv_nsec = -1},
		.ret = -1,
		.err = EINVAL,
		.setup = create_queue,
		.cleanup = unlink_queue,
	},
	{
		.len = 16,
		.rq = &(struct timespec) {.tv_sec = 0, .tv_nsec = 1000000000},
		.ret = -1,
		.err = EINVAL,
		.setup = create_queue,
		.cleanup = unlink_queue,
	},
	{
		.len = 16,
		.ret = -1,
		.rq = &timeout_ts,
		.err = ETIMEDOUT,
		.setup = create_queue_timeout,
		.cleanup = unlink_queue,
	},
	{
		.len = 16,
		.rq = &eintr_ts,
		.ret = -1,
		.err = EINTR,
		.setup = create_queue_sig,
		.cleanup = unlink_queue_sig,
	},
};

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

static void setup(void)
{
	act.sa_handler = sighandler;
	sigaction(SIGINT, &act, NULL);

	fd_root = SAFE_OPEN("/", O_RDONLY);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	if (fd_root > 0)
		SAFE_CLOSE(fd_root);
}

static void create_queue(void)
{
	fd = SAFE_MQ_OPEN(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRWXU, NULL);
}

static void create_queue_nonblock(void)
{
	fd = SAFE_MQ_OPEN(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK,
		S_IRWXU, NULL);
}

static void create_queue_sig(void)
{
	clock_gettime(CLOCK_REALTIME, &eintr_ts);
	eintr_ts.tv_sec += 3;

	create_queue();
	pid = create_sig_proc(SIGINT, 40, 200000);
}

static void create_queue_timeout(void)
{
	clock_gettime(CLOCK_REALTIME, &timeout_ts);
	timeout_ts.tv_nsec += 50000000;
	timeout_ts.tv_sec += timeout_ts.tv_nsec / 1000000000;
	timeout_ts.tv_nsec %= 1000000000;

	create_queue();
}

static void open_fd(void)
{
	fd = fd_root;
}

static void send_msg(int fd, int len, int prio)
{
	char smsg[MAX_MSGSIZE];
	int i;

	for (i = 0; i < len; i++)
		smsg[i] = i;

	if (mq_timedsend(fd, smsg, len, prio,
		&((struct timespec){0})) < 0)
		tst_brk(TBROK | TERRNO, "mq_timedsend failed");
}

static void unlink_queue(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	mq_unlink(QUEUE_NAME);
}

static void unlink_queue_sig(void)
{
	SAFE_KILL(pid, SIGTERM);
	SAFE_WAIT(NULL);

	unlink_queue();
}

static void do_test(unsigned int i)
{
	const struct test_case *tc = &tcase[i];
	char rmsg[MAX_MSGSIZE];
	unsigned prio;
	size_t msg_len = MAX_MSGSIZE;

	/*
	 * When test ended with SIGTERM etc, mq descriptor is left remains.
	 * So we delete it first.
	 */
	mq_unlink(QUEUE_NAME);

	if (tc->fd)
		fd = tc->fd;

	if (tc->setup)
		tc->setup();

	if (tc->send)
		send_msg(fd, tc->len, tc->prio);

	if (tc->invalid_msg)
		msg_len -= 1;

	TEST(mq_timedreceive(fd, rmsg, msg_len, &prio, tc->rq));

	if (tc->cleanup)
		tc->cleanup();

	if (TEST_RETURN < 0) {
		if (TEST_ERRNO != tc->err) {
			tst_res(TFAIL | TTERRNO,
				"mq_timedreceive failed unexpectedly, expected %s",
				tst_strerrno(tc->err));
		} else {
			tst_res(TPASS | TTERRNO, "mq_timedreceive failed expectedly");
		}
		return;
	}


	if (TEST_RETURN != tc->len) {
		tst_res(TFAIL | TTERRNO, "mq_timedreceive wrong msg_len returned %ld, expected %d",
			TEST_RETURN, tc->len);
		return;
	}

	if (tc->prio != prio) {
		tst_res(TFAIL | TTERRNO, "mq_timedreceive wrong prio returned %d, expected %d",
			prio, tc->prio);
		return;
	}

	tst_res(TPASS, "mq_timedreceive returned %ld prio %u", TEST_RETURN, prio);
}

static struct tst_test test = {
	.tid = "mq_timedreceive01",
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
