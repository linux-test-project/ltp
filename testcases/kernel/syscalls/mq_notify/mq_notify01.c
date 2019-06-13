// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007-2008, Hitachi, Ltd
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 *
 * Authors:
 * Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 * Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 * Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 */

#include <limits.h>
#include <errno.h>

#include "tst_test.h"
#include "tst_safe_posix_ipc.h"

static int fd, fd_root, fd_nonblock, fd_maxint = INT_MAX - 1, fd_invalid = -1;

#include "mq.h"

#define USER_DATA       0x12345678

static char *str_debug;

static volatile sig_atomic_t notified, cmp_ok;
static siginfo_t info;

struct test_case {
	int *fd;
	int already_registered;
	int notify;
	int ret;
	int err;
};

static struct test_case tcase[] = {
	{
		.fd = &fd,
		.notify = SIGEV_NONE,
		.ret = 0,
		.err = 0,
	},
	{
		.fd = &fd,
		.notify = SIGEV_SIGNAL,
		.ret = 0,
		.err = 0,
	},
	{
		.fd = &fd,
		.notify = SIGEV_THREAD,
		.ret = 0,
		.err = 0,
	},
	{
		.fd = &fd_invalid,
		.notify = SIGEV_NONE,
		.ret = -1,
		.err = EBADF,
	},
	{
		.fd = &fd_maxint,
		.notify = SIGEV_NONE,
		.ret = -1,
		.err = EBADF,
	},
	{
		.fd = &fd_root,
		.notify = SIGEV_NONE,
		.ret = -1,
		.err = EBADF,
	},
	{
		.fd = &fd,
		.notify = SIGEV_NONE,
		.already_registered = 1,
		.ret = -1,
		.err = EBUSY,
	},
};

static void sigfunc(int signo LTP_ATTRIBUTE_UNUSED, siginfo_t *si,
	void *data LTP_ATTRIBUTE_UNUSED)
{
	if (str_debug)
		memcpy(&info, si, sizeof(info));

	cmp_ok = si->si_code == SI_MESGQ &&
	    si->si_signo == SIGUSR1 &&
	    si->si_value.sival_int == USER_DATA &&
	    si->si_pid == getpid() && si->si_uid == getuid();
	notified = 1;
}

static void tfunc(union sigval sv)
{
	cmp_ok = sv.sival_int == USER_DATA;
	notified = 1;
}

static void do_test(unsigned int i)
{
	struct sigaction sigact;
	struct test_case *tc = &tcase[i];
	struct sigevent ev;

	ev.sigev_notify = tc->notify;
	notified = cmp_ok = 1;

	switch (tc->notify) {
	case SIGEV_SIGNAL:
		notified = cmp_ok = 0;
		ev.sigev_signo = SIGUSR1;
		ev.sigev_value.sival_int = USER_DATA;

		memset(&sigact, 0, sizeof(sigact));
		sigact.sa_sigaction = sigfunc;
		sigact.sa_flags = SA_SIGINFO;
		if (sigaction(SIGUSR1, &sigact, NULL) == -1) {
			tst_res(TFAIL | TTERRNO, "sigaction failed");
			return;
		}
		break;
	case SIGEV_THREAD:
		notified = cmp_ok = 0;
		ev.sigev_notify_function = tfunc;
		ev.sigev_notify_attributes = NULL;
		ev.sigev_value.sival_int = USER_DATA;
		break;
	}

	if (tc->already_registered && mq_notify(*tc->fd, &ev) == -1) {
		tst_res(TFAIL | TERRNO, "mq_notify(%d, %p) failed", fd, &ev);
		return;
	}

	TEST(mq_notify(*tc->fd, &ev));

	if (TST_RET < 0) {
		if (tc->err != TST_ERR)
			tst_res(TFAIL | TTERRNO,
				"mq_notify failed unexpectedly, expected %s",
				tst_strerrno(tc->err));
		else
			tst_res(TPASS | TTERRNO, "mq_notify failed expectedly");

		/* unregister notification */
		if (*tc->fd == fd)
			mq_notify(*tc->fd, NULL);

		return;
	}

	TEST(mq_timedsend(*tc->fd, smsg, MSG_LENGTH, 0,
		&((struct timespec){0})));

	if (*tc->fd == fd)
		cleanup_queue(fd);

	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "mq_timedsend failed");
		return;
	}

	while (!notified)
		usleep(10000);

	if (str_debug && tc->notify == SIGEV_SIGNAL) {
		tst_res(TINFO, "si_code  E:%d,\tR:%d",
			info.si_code, SI_MESGQ);
		tst_res(TINFO, "si_signo E:%d,\tR:%d",
			info.si_signo, SIGUSR1);
		tst_res(TINFO, "si_value E:0x%x,\tR:0x%x",
			info.si_value.sival_int, USER_DATA);
		tst_res(TINFO, "si_pid   E:%d,\tR:%d",
			info.si_pid, getpid());
		tst_res(TINFO, "si_uid   E:%d,\tR:%d",
			info.si_uid, getuid());
	}

	if (TST_RET < 0) {
		if (tc->err != TST_ERR)
			tst_res(TFAIL | TTERRNO,
				"mq_timedsend failed unexpectedly, expected %s",
				tst_strerrno(tc->err));
		else
			tst_res(TPASS | TTERRNO, "mq_timedsend failed expectedly");
		return;
	}

	if (tc->ret != TST_RET) {
		tst_res(TFAIL, "mq_timedsend returned %ld, expected %d",
			TST_RET, tc->ret);
		return;
	}

	tst_res(TPASS, "mq_notify and mq_timedsend exited expectedly");
}

static struct tst_option options[] = {
	{"d", &str_debug, "Print debug messages"},
	{NULL, NULL, NULL}
};

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.options = options,
	.setup = setup_common,
	.cleanup = cleanup_common,
};
