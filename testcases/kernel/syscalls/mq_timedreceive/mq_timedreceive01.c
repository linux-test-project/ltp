/*
 * Copyright (c) Crackerjack Project., 2007-2008, Hitachi, Ltd
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 *
 * Authors:
 * Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 * Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 * Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <limits.h>

static int fd, fd_root, fd_nonblock, fd_maxint = INT_MAX - 1, fd_invalid = -1;

#include "mq_timed.h"

static struct timespec ts;

static struct test_case tcase[] = {
	{
		.fd = &fd,
		.len = 0,
		.send = 1,
		.ret = 0,
		.err = 0,
	},
	{
		.fd = &fd,
		.len = 1,
		.send = 1,
		.ret = 0,
		.err = 0,
	},
	{
		.fd = &fd,
		.len = MAX_MSGSIZE,
		.send = 1,
		.ret = 0,
		.err = 0,
	},
	{
		.fd = &fd,
		.len = 1,
		.send = 1,
		.prio = MQ_PRIO_MAX - 1,
		.ret = 0,
		.err = 0,
	},
	{
		.fd = &fd,
		.invalid_msg = 1,
		.len = 0,
		.send = 1,
		.ret = -1,
		.err = EMSGSIZE,
	},
	{
		.fd = &fd_invalid,
		.len = 0,
		.ret = -1,
		.err = EBADF,
	},
	{
		.fd = &fd_maxint,
		.len = 0,
		.ret = -1,
		.err = EBADF,
	},
	{
		.fd = &fd_root,
		.len = 0,
		.ret = -1,
		.err = EBADF,
	},
	{
		.fd = &fd_nonblock,
		.len = 16,
		.ret = -1,
		.err = EAGAIN,
	},
	{
		.fd = &fd,
		.len = 16,
		.rq = &(struct timespec) {.tv_sec = -1, .tv_nsec = 0},
		.ret = -1,
		.err = EINVAL,
	},
	{
		.fd = &fd,
		.len = 16,
		.rq = &(struct timespec) {.tv_sec = 0, .tv_nsec = -1},
		.ret = -1,
		.err = EINVAL,
	},
	{
		.fd = &fd,
		.len = 16,
		.rq = &(struct timespec) {.tv_sec = 0, .tv_nsec = 1000000000},
		.ret = -1,
		.err = EINVAL,
	},
	{
		.fd = &fd,
		.len = 16,
		.timeout = 1,
		.ret = -1,
		.rq = &ts,
		.err = ETIMEDOUT,
	},
	{
		.fd = &fd,
		.len = 16,
		.signal = 1,
		.rq = &ts,
		.ret = -1,
		.err = EINTR,
	},
};

static void do_test(unsigned int i)
{
	const struct test_case *tc = &tcase[i];
	unsigned int j;
	unsigned int prio;
	size_t len = MAX_MSGSIZE;
	char rmsg[len];
	pid_t pid = -1;

	if (tc->signal)
		pid = set_sig(tc->rq);

	if (tc->timeout)
		set_timeout(tc->rq);

	if (tc->send)
		send_msg(*tc->fd, tc->len, tc->prio);

	if (tc->invalid_msg)
		len -= 1;

	TEST(mq_timedreceive(*tc->fd, rmsg, len, &prio, tc->rq));

	if (pid > 0)
		kill_pid(pid);

	if (*tc->fd == fd)
		cleanup_queue(fd);

	if (TST_RET < 0) {
		if (tc->err != TST_ERR)
			tst_res(TFAIL | TTERRNO,
				"mq_timedreceive failed unexpectedly, expected %s",
				tst_strerrno(tc->err));
		else
			tst_res(TPASS | TTERRNO, "mq_timedreceive failed expectedly");

		return;
	}

	if (tc->len != TST_RET) {
		tst_res(TFAIL, "mq_timedreceive wrong length %ld, expected %zu",
			TST_RET, tc->len);
		return;
	}

	if (tc->prio != prio) {
		tst_res(TFAIL, "mq_timedreceive wrong prio %d, expected %d",
			prio, tc->prio);
		return;
	}

	for (j = 0; j < tc->len; j++) {
		if (rmsg[j] != smsg[j]) {
			tst_res(TFAIL,
				"mq_timedreceive wrong data %d in %u, expected %d",
				rmsg[j], i, smsg[j]);
			return;
		}
	}

	tst_res(TPASS, "mq_timedreceive returned %ld, priority %u, length: %zu",
			TST_RET, prio, len);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.setup = setup_common,
	.cleanup = cleanup_common,
	.forks_child = 1,
};
