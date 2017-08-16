/*
 * Copyright (c) 2014 SUSE Linux.  All Rights Reserved.
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
 * Started by Jan Kara <jack@suse.cz>
 *
 * DESCRIPTION
 *     Check that fanotify overflow event is properly generated
 *
 * ALGORITHM
 *     Generate enough events without reading them and check that overflow
 *     event is generated.
 */
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

/* Currently this is fixed in kernel... */
#define MAX_EVENTS 16384

#define BUF_SIZE 256
static char fname[BUF_SIZE];
static int fd, fd_notify;

struct fanotify_event_metadata event;

void test01(void)
{
	int i;
	int len;

	/*
	 * generate events
	 */
	for (i = 0; i < MAX_EVENTS + 1; i++) {
		sprintf(fname, "fname_%d", i);
		fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0644);
		SAFE_CLOSE(fd);
	}

	while (1) {
		/*
		 * get list on events
		 */
		len = read(fd_notify, &event, sizeof(event));
		if (len < 0) {
			if (errno == -EAGAIN) {
				tst_res(TFAIL, "Overflow event not "
					 "generated!\n");
				break;
			}
			tst_brk(TBROK | TERRNO,
				 "read of notification event failed");
			break;
		}
		if (event.fd != FAN_NOFD)
			close(event.fd);

		/*
		 * check events
		 */
		if (event.mask != FAN_OPEN &&
		    event.mask != FAN_Q_OVERFLOW) {
			tst_res(TFAIL,
				 "get event: mask=%llx (expected %llx)"
				 "pid=%u fd=%d",
				 (unsigned long long)event.mask,
				 (unsigned long long)FAN_OPEN,
				 (unsigned)event.pid, event.fd);
			break;
		}
		if (event.mask == FAN_Q_OVERFLOW) {
			if (event.fd != FAN_NOFD) {
				tst_res(TFAIL,
					 "invalid overflow event: "
					 "mask=%llx pid=%u fd=%d",
					 (unsigned long long)event.mask,
					 (unsigned)event.pid,
					 event.fd);
				break;
			}
			tst_res(TPASS,
				 "get event: mask=%llx pid=%u fd=%d",
				 (unsigned long long)event.mask,
				 (unsigned)event.pid, event.fd);
				break;
		}
	}
}

static void setup(void)
{
	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF | FAN_NONBLOCK,
			O_RDONLY);

	if (fanotify_mark(fd_notify, FAN_MARK_MOUNT | FAN_MARK_ADD, FAN_OPEN,
			    AT_FDCWD, ".") < 0) {
		tst_brk(TBROK | TERRNO,
			 "fanotify_mark (%d, FAN_MARK_MOUNT | FAN_MARK_ADD, "
			 "FAN_OPEN, AT_FDCWD, \".\") failed",
			 fd_notify);
	}
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.test_all = test01,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
};
#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
