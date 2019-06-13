// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2007 SWSoft.  All Rights Reserved.
 * Author: Andrew Vagin <avagin@sw.ru>
 *
 * DESCRIPTION
 *     Check that inotify work for a directory
 *
 * ALGORITHM
 *     Execute sequence file's operation and check return events
 */

#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <limits.h>
#include "tst_test.h"
#include "inotify.h"

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>

#ifndef IN_MOVE_SELF
#define IN_MOVE_SELF            0x00000800
#endif

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct inotify_event))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * (EVENT_SIZE + 16))

#define BUF_SIZE 256
static char fname1[BUF_SIZE], fname2[BUF_SIZE], fname3[BUF_SIZE];
static int fd, fd_notify, reap_wd;
static int wd;

struct event_t {
	char name[BUF_SIZE];
	unsigned int mask;
};
#define FILE_NAME1 "test_file1"
#define FILE_NAME2 "test_file2"

static struct event_t event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

void verify_inotify(void)
{
	unsigned int stored_cookie = UINT_MAX;

	int test_cnt = 0;

	/*
	 * generate sequence of events
	 */
	SAFE_CHMOD(".", 0755);
	event_set[test_cnt].mask = IN_ISDIR | IN_ATTRIB;
	strcpy(event_set[test_cnt].name, "");
	test_cnt++;

	if ((fd = creat(FILE_NAME1, 0755)) == -1) {
		tst_brk(TBROK | TERRNO,
			"creat(\"%s\", 755) failed", FILE_NAME1);
	}

	event_set[test_cnt].mask = IN_CREATE;
	strcpy(event_set[test_cnt].name, FILE_NAME1);
	test_cnt++;
	event_set[test_cnt].mask = IN_OPEN;
	strcpy(event_set[test_cnt].name, FILE_NAME1);
	test_cnt++;

	SAFE_CLOSE(fd);
	event_set[test_cnt].mask = IN_CLOSE_WRITE;
	strcpy(event_set[test_cnt].name, FILE_NAME1);
	test_cnt++;

	SAFE_RENAME(FILE_NAME1, FILE_NAME2);
	event_set[test_cnt].mask = IN_MOVED_FROM;
	strcpy(event_set[test_cnt].name, FILE_NAME1);
	test_cnt++;
	event_set[test_cnt].mask = IN_MOVED_TO;
	strcpy(event_set[test_cnt].name, FILE_NAME2);
	test_cnt++;

	if (getcwd(fname1, BUF_SIZE) == NULL) {
		tst_brk(TBROK | TERRNO,
			"getcwd(%p, %d) failed", fname1, BUF_SIZE);
	}

	snprintf(fname2, BUF_SIZE, "%s.rename1", fname1);
	SAFE_RENAME(fname1, fname2);
	event_set[test_cnt].mask = IN_MOVE_SELF;
	strcpy(event_set[test_cnt].name, "");
	test_cnt++;

	SAFE_UNLINK(FILE_NAME2);
	event_set[test_cnt].mask = IN_DELETE;
	strcpy(event_set[test_cnt].name, FILE_NAME2);
	test_cnt++;

	/*
	 * test that duplicate events will be coalesced into
	 * a single event. This test case should be last, that
	 * we can correct determine kernel bug which exist before
	 * 2.6.25. See comment below.
	 */
	snprintf(fname3, BUF_SIZE, "%s.rename2", fname1);
	SAFE_RENAME(fname2, fname3);

	SAFE_RENAME(fname3, fname1);
	event_set[test_cnt].mask = IN_MOVE_SELF;
	strcpy(event_set[test_cnt].name, "");
	test_cnt++;

	int len, i = 0, test_num = 0;
	if ((len = read(fd_notify, event_buf, EVENT_BUF_LEN)) == -1) {
		tst_brk(TBROK | TERRNO,
			"read(%d, buf, %zu) failed",
			fd_notify, EVENT_BUF_LEN);

	}

	while (i < len) {
		struct inotify_event *event;
		event = (struct inotify_event *)&event_buf[i];
		if (test_num >= test_cnt) {
			if (tst_kvercmp(2, 6, 25) < 0
					&& event_set[test_cnt - 1].mask ==
					event->mask)
				tst_res(TWARN,
					"This may be kernel bug. "
					"Before kernel 2.6.25, a kernel bug "
					"meant that the kernel code that was "
					"intended to coalesce successive identical "
					"events (i.e., the two most recent "
					"events could potentially be coalesced "
					"if the older had not yet been read) "
					"instead checked if the most recent event "
					"could be coalesced with the oldest "
					"unread event. This has been fixed by commit"
					"1c17d18e3775485bf1e0ce79575eb637a94494a2.");
			tst_res(TFAIL,
				"get unnecessary event: "
				"wd=%d mask=%08x cookie=%-5u len=%-2u"
				"name=\"%.*s\"", event->wd, event->mask,
				event->cookie, event->len, event->len,
				event->name);

		} else if ((event_set[test_num].mask == event->mask)
				&&
				(!strncmp
				 (event_set[test_num].name, event->name,
				  event->len))) {
			int fail = 0;

			if (event->mask == IN_MOVED_FROM) {
				if (event->cookie == 0)
					fail = 1;
				else
					stored_cookie = event->cookie;
			} else if (event->mask == IN_MOVED_TO) {
				if (event->cookie != stored_cookie)
					fail = 1;
				else
					stored_cookie = UINT_MAX;
			} else {
				if (event->cookie != 0)
					fail = 1;
			}
			if (!fail) {
				tst_res(TPASS,
					"get event: wd=%d mask=%08x "
					"cookie=%-5u len=%-2u name=\"%.*s\"",
					event->wd, event->mask,
					event->cookie, event->len,
					event->len, event->name);
			} else {
				tst_res(TFAIL,
					"get event: wd=%d mask=%08x "
					"cookie=%-5u (wrong) len=%-2u "
					"name=\"%s\"",
					event->wd, event->mask,
					event->cookie, event->len,
					event->name);
			}
		} else {
			tst_res(TFAIL, "get event: wd=%d mask=%08x "
				"(expected %x) cookie=%-5u len=%-2u "
				"name=\"%s\" (expected \"%s\") %d",
				event->wd, event->mask,
				event_set[test_num].mask,
				event->cookie, event->len, event->name,
				event_set[test_num].name,
				strcmp(event_set[test_num].name,
					event->name));
		}
		test_num++;
		i += EVENT_SIZE + event->len;
	}

	for (; test_num < test_cnt; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%08x ",
			event_set[test_num].mask);
	}
}

static void setup(void)
{
	fd_notify = SAFE_MYINOTIFY_INIT();

	wd = SAFE_MYINOTIFY_ADD_WATCH(fd_notify, ".", IN_ALL_EVENTS);
	reap_wd = 1;
}

static void cleanup(void)
{
	if (reap_wd && myinotify_rm_watch(fd_notify, wd) < 0) {
		tst_res(TWARN,
			"inotify_rm_watch (%d, %d) failed,", fd_notify, wd);
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
