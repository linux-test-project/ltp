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
 *     Check that inotify overflow event is properly generated
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
#include "inotify.h"

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>

/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (EVENT_SIZE * 16)

#define BUF_SIZE 256
static char fname[BUF_SIZE];
static char buf[BUF_SIZE];
static int fd, fd_notify;
static int wd;
static int max_events;

static char event_buf[EVENT_BUF_LEN];

void verify_inotify(void)
{
	int i;
	int len, stop;

	/*
	 * generate events
	 */
	fd = SAFE_OPEN(fname, O_RDWR);

	for (i = 0; i < max_events; i++) {
		SAFE_LSEEK(fd, 0, SEEK_SET);
		SAFE_READ(1, fd, buf, BUF_SIZE);
		SAFE_LSEEK(fd, 0, SEEK_SET);
		SAFE_WRITE(1, fd, buf, BUF_SIZE);
	}

	SAFE_CLOSE(fd);

	stop = 0;
	while (!stop) {
		/*
		 * get list on events
		 */
		len = read(fd_notify, event_buf, EVENT_BUF_LEN);
		if (len < 0) {
			tst_brk(TBROK | TERRNO,
				"read(%d, buf, %zu) failed",
				fd_notify, EVENT_BUF_LEN);
		}

		/*
		 * check events
		 */
		i = 0;
		while (i < len) {
			struct inotify_event *event;

			event = (struct inotify_event *)&event_buf[i];
			if (event->mask != IN_ACCESS &&
					event->mask != IN_MODIFY &&
					event->mask != IN_OPEN &&
					event->mask != IN_Q_OVERFLOW) {
				tst_res(TFAIL,
					"get event: wd=%d mask=%x "
					"cookie=%u (expected 0) len=%u",
					event->wd, event->mask,
					event->cookie, event->len);
				stop = 1;
				break;
			}
			if (event->mask == IN_Q_OVERFLOW) {
				if (event->len != 0 ||
						event->cookie != 0 ||
						event->wd != -1) {
					tst_res(TFAIL,
						"invalid overflow event: "
						"wd=%d mask=%x "
						"cookie=%u len=%u",
						event->wd, event->mask,
						event->cookie,
						event->len);
					stop = 1;
					break;
				}
				if ((int)(i + EVENT_SIZE) != len) {
					tst_res(TFAIL,
						"overflow event is not last");
					stop = 1;
					break;
				}
				tst_res(TPASS, "get event: wd=%d "
					"mask=%x cookie=%u len=%u",
					event->wd, event->mask,
					event->cookie, event->len);
				stop = 1;
				break;
			}
			i += EVENT_SIZE + event->len;
		}
	}
}

static void setup(void)
{
	sprintf(fname, "tfile_%d", getpid());
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
	SAFE_WRITE(1, fd, buf, BUF_SIZE);
	SAFE_CLOSE(fd);

	fd_notify = myinotify_init1(O_NONBLOCK);
	if (fd_notify < 0) {
		if (errno == ENOSYS) {
			tst_brk(TCONF,
				"inotify is not configured in this kernel.");
		} else {
			tst_brk(TBROK | TERRNO,
				"inotify_init failed");
		}
	}

	wd = myinotify_add_watch(fd_notify, fname, IN_ALL_EVENTS);
	if (wd < 0) {
		tst_brk(TBROK | TERRNO,
			"inotify_add_watch (%d, %s, IN_ALL_EVENTS) failed",
			fd_notify, fname);
	};

	SAFE_FILE_SCANF("/proc/sys/fs/inotify/max_queued_events",
			"%d", &max_events);
}

static void cleanup(void)
{
	if (fd_notify > 0 && myinotify_rm_watch(fd_notify, wd) == -1) {
		tst_res(TWARN | TERRNO, "inotify_rm_watch (%d, %d) failed",
			fd_notify, wd);

	}

	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_inotify,
};

#else
	TST_TEST_TCONF("system doesn't have required inotify support");
#endif
