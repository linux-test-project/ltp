// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2007 SWSoft.  All Rights Reserved.
 * Author: Andrew Vagin <avagin@sw.ru>
 */

/*\
 * [Description]
 *
 * Basic test for inotify events on file.
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

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct inotify_event))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * (EVENT_SIZE + 16))

#define BUF_SIZE 256

static char fname[BUF_SIZE];
static char buf[BUF_SIZE];
static int fd, fd_notify;
static int wd, reap_wd;

static unsigned int event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

void verify_inotify(void)
{
	int test_cnt = 0;

	/*
	 * generate sequence of events
	 */
	SAFE_CHMOD(fname, 0755);
	event_set[test_cnt] = IN_ATTRIB;
	test_cnt++;

	fd = SAFE_OPEN(fname, O_RDONLY);
	event_set[test_cnt] = IN_OPEN;
	test_cnt++;

	if (read(fd, buf, BUF_SIZE) == -1) {
		tst_brk(TBROK | TERRNO,
			"read(%d, buf, %d) failed", fd, BUF_SIZE);
	}
	event_set[test_cnt] = IN_ACCESS;
	test_cnt++;

	SAFE_CLOSE(fd);
	event_set[test_cnt] = IN_CLOSE_NOWRITE;
	test_cnt++;

	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
	event_set[test_cnt] = IN_OPEN;
	test_cnt++;

	if (write(fd, buf, BUF_SIZE) == -1) {
		tst_brk(TBROK,
			"write(%d, %s, 1) failed", fd, fname);
	}
	event_set[test_cnt] = IN_MODIFY;
	test_cnt++;

	SAFE_CLOSE(fd);
	event_set[test_cnt] = IN_CLOSE_WRITE;
	test_cnt++;

	/*
	 * get list of events
	 */
	int len, i = 0, test_num = 0;
	if ((len = read(fd_notify, event_buf, EVENT_BUF_LEN)) < 0) {
		tst_brk(TBROK,
			"read(%d, buf, %zu) failed",
			fd_notify, EVENT_BUF_LEN);

	}

	/*
	 * check events
	 */
	while (i < len) {
		struct inotify_event *event;
		event = (struct inotify_event *)&event_buf[i];
		if (test_num >= test_cnt) {
			tst_res(TFAIL,
				"get unnecessary event: wd=%d mask=%02x "
				"cookie=%u len=%u",
				event->wd, event->mask,
				event->cookie, event->len);
		} else if (event_set[test_num] == event->mask) {
			if (event->cookie != 0) {
				tst_res(TFAIL,
					"get event: wd=%d mask=%02x "
					"cookie=%u (expected 0) len=%u",
					event->wd, event->mask,
					event->cookie, event->len);
			} else {
				tst_res(TPASS, "get event: wd=%d "
					"mask=%02x cookie=%u len=%u",
					event->wd, event->mask,
					event->cookie, event->len);
			}

		} else {
			tst_res(TFAIL, "get event: wd=%d mask=%02x "
				"(expected %x) cookie=%u len=%u",
				event->wd, event->mask,
				event_set[test_num],
				event->cookie, event->len);
		}
		test_num++;
		i += EVENT_SIZE + event->len;
	}
	for (; test_num < test_cnt; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%02x",
			event_set[test_num]);

	}
}

static void setup(void)
{
	sprintf(fname, "tfile_%d", getpid());

	SAFE_FILE_PRINTF(fname, "%s", fname);

	fd_notify = SAFE_MYINOTIFY_INIT();

	wd = SAFE_MYINOTIFY_ADD_WATCH(fd_notify, fname, IN_ALL_EVENTS);
	reap_wd = 1;
}

static void cleanup(void)
{
	if (reap_wd && myinotify_rm_watch(fd_notify, wd) < 0) {
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
