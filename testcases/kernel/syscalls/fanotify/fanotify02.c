// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 SUSE.  All Rights Reserved.
 *
 * Started by Jan Kara <jack@suse.cz>
 */

/*\
 * [Description]
 * Check that fanotify work for children of a directory.
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

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof(struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)

#define BUF_SIZE 256
#define TST_TOTAL 8

static char fname[BUF_SIZE];
static char buf[BUF_SIZE];
static int fd, fd_notify;

static unsigned long long event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

static void test01(void)
{
	int ret, len, i = 0, test_num = 0;

	int tst_count = 0;

	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD, FAN_ACCESS |
			  FAN_MODIFY | FAN_CLOSE | FAN_OPEN | FAN_EVENT_ON_CHILD |
			  FAN_ONDIR, AT_FDCWD, ".");

	/*
	 * generate sequence of events
	 */
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
	event_set[tst_count] = FAN_OPEN;
	tst_count++;

	SAFE_WRITE(SAFE_WRITE_ALL, fd, fname, strlen(fname));
	event_set[tst_count] = FAN_MODIFY;
	tst_count++;

	SAFE_CLOSE(fd);
	event_set[tst_count] = FAN_CLOSE_WRITE;
	tst_count++;

	/*
	 * Get list of events so far. We get events here to avoid
	 * merging of following events with the previous ones.
	 */
	ret = SAFE_READ(0, fd_notify, event_buf,
			EVENT_BUF_LEN);
	len = ret;

	fd = SAFE_OPEN(fname, O_RDONLY);
	event_set[tst_count] = FAN_OPEN;
	tst_count++;

	SAFE_READ(0, fd, buf, BUF_SIZE);
	event_set[tst_count] = FAN_ACCESS;
	tst_count++;

	SAFE_CLOSE(fd);
	event_set[tst_count] = FAN_CLOSE_NOWRITE;
	tst_count++;

	/*
	 * get next events
	 */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	/*
	 * now remove child mark
	 */
	SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_REMOVE,
			  FAN_EVENT_ON_CHILD, AT_FDCWD, ".");

	/*
	 * Do something to verify events didn't get generated
	 */
	fd = SAFE_OPEN(fname, O_RDONLY);

	SAFE_CLOSE(fd);

	fd = SAFE_OPEN(".", O_RDONLY | O_DIRECTORY);
	event_set[tst_count] = FAN_OPEN;
	tst_count++;

	SAFE_CLOSE(fd);
	event_set[tst_count] = FAN_CLOSE_NOWRITE;
	tst_count++;

	/*
	 * Check events got generated only for the directory
	 */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	if (TST_TOTAL != tst_count) {
		tst_brk(TBROK,
			"TST_TOTAL and tst_count are not equal");
	}
	tst_count = 0;

	/*
	 * check events
	 */
	while (i < len) {
		struct fanotify_event_metadata *event;

		event = (struct fanotify_event_metadata *)&event_buf[i];
		if (test_num >= TST_TOTAL) {
			tst_res(TFAIL,
				"get unnecessary event: mask=%llx "
				"pid=%u fd=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd);
		} else if (!(event->mask & event_set[test_num])) {
			tst_res(TFAIL,
				"got event: mask=%llx (expected %llx) "
				"pid=%u fd=%d",
				(unsigned long long)event->mask,
				event_set[test_num],
				(unsigned int)event->pid, event->fd);
		} else if (event->pid != getpid()) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u "
				"(expected %u) fd=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid,
				(unsigned int)getpid(),
				event->fd);
		} else {
			tst_res(TPASS,
				"got event: mask=%llx pid=%u fd=%u",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd);
		}
		event->mask &= ~event_set[test_num];
		/* No events left in current mask? Go for next event */
		if (event->mask == 0) {
			i += event->event_len;
			if (event->fd != FAN_NOFD)
				SAFE_CLOSE(event->fd);
		}
		test_num++;
	}
	for (; test_num < TST_TOTAL; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%llx",
			event_set[test_num]);

	}
}

static void setup(void)
{
	sprintf(fname, "fname_%d", getpid());
	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF, O_RDONLY);
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
	.needs_tmpdir = 1,
	.needs_root = 1
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
