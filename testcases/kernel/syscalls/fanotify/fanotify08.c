// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2017 RedHat.  All Rights Reserved.
 *
 * Started by Xiong Zhou <xzhou@redhat.com>
 */

/*\
 * [Description]
 * Sanity check fanotify_init flag FAN_CLOEXEC by fcntl.
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include "tst_test.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

static int fd_notify;

static void test_init_bit(unsigned int fan_bit,
			unsigned int fd_bit, char *msg)
{
	int ret;

	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF|fan_bit, O_RDONLY);

	ret = SAFE_FCNTL(fd_notify, F_GETFD);

	if ((ret & FD_CLOEXEC) == fd_bit)
		tst_res(TPASS, "%s", msg);
	else
		tst_res(TFAIL, "%s", msg);

	SAFE_CLOSE(fd_notify);
}

static void run(unsigned int i)
{
	switch (i) {
	case 0:
		test_init_bit(0, 0, "not set close_on_exec");
		break;
	case 1:
		test_init_bit(FAN_CLOEXEC, FD_CLOEXEC, "set close_on_exec");
		break;
	}
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
	.cleanup = cleanup,
	.needs_root = 1,
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
