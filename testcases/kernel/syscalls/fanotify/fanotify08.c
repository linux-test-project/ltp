/*
 * Copyright (c) 2017 RedHat.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Started by Xiong Zhou <xzhou@redhat.com>
 *
 * DESCRIPTION
 *     Sanity check fanotify_init flag FAN_CLOEXEC by fcntl.
 */
#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "fanotify.h"

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>

static int fd_notify;

static void test_init_bit(unsigned int fan_bit,
			unsigned int fd_bit, char *msg)
{
	int ret;

	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF|fan_bit, O_RDONLY);

	ret = SAFE_FCNTL(fd_notify, F_GETFD);

	if ((ret & FD_CLOEXEC) == fd_bit) {
		tst_res(TPASS, msg);
	} else {
		tst_res(TFAIL, msg);
	}

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
