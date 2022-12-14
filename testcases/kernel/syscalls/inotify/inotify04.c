// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2012 Linux Test Project.  All Rights Reserved.
 * Ngie Cooper, April 2012
 *
 * DESCRIPTION
 *     verify that IN_DELETE_SELF functions as expected
 *
 * ALGORITHM
 *     This testcase creates a temporary directory, then add watches to a
 *     predefined file and subdirectory, and delete the file and directory to
 *     ensure that the IN_DELETE_SELF event is captured properly.
 *
 *     Because of how the inotify(7) API is designed, we also need to catch the
 *     IN_ATTRIB and IN_IGNORED events.
 */

#include "config.h"

#if defined(HAVE_SYS_INOTIFY_H)
# include <sys/inotify.h>
#endif
#include <errno.h>
#include <string.h>
#include "tst_test.h"
#include "inotify.h"

#if defined(HAVE_SYS_INOTIFY_H)

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof(struct inotify_event))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * (EVENT_SIZE + 16))


#define BUF_SIZE 256

struct event_t {
	char name[BUF_SIZE];
	unsigned int mask;
};

#define	TEST_DIR	"test_dir"
#define	TEST_FILE	"test_file"

struct event_t event_set[EVENT_MAX];

char event_buf[EVENT_BUF_LEN];

int fd_notify, reap_wd_file, reap_wd_dir, wd_dir, wd_file;

static void cleanup(void)
{
	if (reap_wd_dir && myinotify_rm_watch(fd_notify, wd_dir) == -1)
		tst_res(TWARN,
			"inotify_rm_watch(%d, %d) [1] failed", fd_notify,
			wd_dir);

	if (reap_wd_file && myinotify_rm_watch(fd_notify, wd_file) == -1)
		tst_res(TWARN,
			"inotify_rm_watch(%d, %d) [2] failed", fd_notify,
			wd_file);

	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static void setup(void)
{
	fd_notify = SAFE_MYINOTIFY_INIT();
}

void verify_inotify(void)
{
	int i = 0, test_num = 0, len;
	int test_cnt = 0;

	SAFE_MKDIR(TEST_DIR, 00700);
	close(SAFE_CREAT(TEST_FILE, 00600));

	wd_dir = SAFE_MYINOTIFY_ADD_WATCH(fd_notify, TEST_DIR, IN_ALL_EVENTS);
	reap_wd_dir = 1;

	wd_file = SAFE_MYINOTIFY_ADD_WATCH(fd_notify, TEST_FILE, IN_ALL_EVENTS);
	reap_wd_file = 1;

	SAFE_RMDIR(TEST_DIR);
	reap_wd_dir = 0;

	event_set[test_cnt].mask = IN_DELETE_SELF;
	strcpy(event_set[test_cnt].name, "");
	test_cnt++;
	event_set[test_cnt].mask = IN_IGNORED;
	strcpy(event_set[test_cnt].name, "");
	test_cnt++;

	SAFE_UNLINK(TEST_FILE);
	reap_wd_file = 0;

	/*
	 * When a file is unlinked, the link count is reduced by 1, and when it
	 * hits 0 the file is removed.
	 *
	 * This isn't well documented in inotify(7), but it's intuitive if you
	 * understand how Unix works.
	 */
	event_set[test_cnt].mask = IN_ATTRIB;
	strcpy(event_set[test_cnt].name, "");
	test_cnt++;

	event_set[test_cnt].mask = IN_DELETE_SELF;
	strcpy(event_set[test_cnt].name, TEST_FILE);
	test_cnt++;
	event_set[test_cnt].mask = IN_IGNORED;
	strcpy(event_set[test_cnt].name, "");
	test_cnt++;

	len = read(fd_notify, event_buf, EVENT_BUF_LEN);
	if (len == -1)
		tst_brk(TBROK | TERRNO, "read failed");

	while (i < len) {
		struct inotify_event *event;
		event = (struct inotify_event *)&event_buf[i];
		if (test_num >= test_cnt) {
			if (tst_kvercmp(2, 6, 25) < 0
			    && event_set[test_cnt - 1].mask == event->mask)
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
				"got unnecessary event: "
				"wd=%d mask=%04x cookie=%u len=%u "
				"name=\"%.*s\"", event->wd, event->mask,
				event->cookie, event->len, event->len, event->name);

		} else if ((event_set[test_num].mask == event->mask)
			   &&
			   (!strncmp
			    (event_set[test_num].name, event->name,
			     event->len))) {
			tst_res(TPASS,
				"got event: wd=%d mask=%04x "
				"cookie=%u len=%u name=\"%.*s\"",
				event->wd, event->mask, event->cookie,
				event->len, event->len, event->name);

		} else {
			tst_res(TFAIL, "got event: wd=%d mask=%04x "
				"(expected %x) cookie=%u len=%u "
				"name=\"%.*s\" (expected \"%s\") %d",
				event->wd, event->mask,
				event_set[test_num].mask,
				event->cookie, event->len,
				event->len, event->name,
				event_set[test_num].name,
				strncmp(event_set[test_num].name, event->name, event->len));
		}
		test_num++;
		i += EVENT_SIZE + event->len;
	}

	for (; test_num < test_cnt; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%04x ",
			event_set[test_num].mask);
	}

}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_inotify,
};

#else
	TST_TEST_TCONF("system doesn't have required inotify support");
#endif /* defined(HAVE_SYS_INOTIFY_H) */
