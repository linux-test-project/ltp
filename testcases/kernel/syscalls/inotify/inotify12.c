// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 CTERA Networks. All Rights Reserved.
 *
 * Author: Amir Goldstein <amir73il@gmail.com>
 */

/*\
 * Test special inotify mask flags.
 *
 * Regression test for kernel commit:
 * a32e697cda27 ("inotify: show inotify mask flags in proc fdinfo").
 */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "tst_safe_macros.h"
#include "inotify.h"

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>

#define EVENT_MAX 32
/* Size of the event structure, not including the name */
#define EVENT_SIZE	(sizeof(struct inotify_event))
#define EVENT_BUF_LEN	(EVENT_MAX * (EVENT_SIZE + 16))

#define	TEST_FILE	"test_file"

static char event_buf[EVENT_BUF_LEN];

static struct tcase {
	const char *tname;
	unsigned int mask;
	int expect_events;
} tcases[] = {
	{
		"Watch for multi events",
		IN_MODIFY,
		2,
	},
	{
		"Watch for single event",
		IN_MODIFY | IN_ONESHOT,
		1,
	},
	{
		"Watch for events on linked file",
		IN_MODIFY | IN_EXCL_UNLINK,
		1,
	},
};

static int fd_notify;

static void verify_inotify(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fd, len;
	unsigned int tmpmask;
	char procfdinfo[100];
	struct inotify_event *event = (struct inotify_event *)event_buf;

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	fd_notify = SAFE_MYINOTIFY_INIT1(O_NONBLOCK);

	SAFE_FILE_PRINTF(TEST_FILE, "1");

	SAFE_MYINOTIFY_ADD_WATCH(fd_notify, ".", tc->mask);

	sprintf(procfdinfo, "/proc/%d/fdinfo/%d", (int)getpid(), fd_notify);
	if (FILE_LINES_SCANF(procfdinfo, "inotify wd:%*d ino:%*x sdev:%*x mask:%x",
			     &tmpmask)) {
		tst_res(TFAIL, "Could not parse inotify fdinfo");
	} else if (tmpmask != tc->mask) {
		tst_res(TFAIL, "Incorrect mask %x in inotify fdinfo (expected %x)",
			tmpmask, tc->mask);
	} else {
		tst_res(TPASS, "Correct mask in inotify fdinfo");
	}

	fd = SAFE_OPEN(TEST_FILE, O_RDWR);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, "2", 1);

	/*
	 * Read the 1st IN_MODIFY event
	 */
	len = SAFE_READ(0, fd_notify, event_buf, EVENT_BUF_LEN);

	if (len < (int)sizeof(*event)) {
		tst_res(TFAIL, "Got no events");
	} else if (event->mask == IN_MODIFY) {
		tst_res(TPASS, "Got 1st event as expected");
	} else {
		tst_res(TFAIL, "Got event 0x%x (expected 0x%x)",
				event->mask, IN_MODIFY);
	}

	/*
	 * Unlink file so IN_EXCL_UNLINK won't get IN_ACCESS event.
	 * IN_ONESHOT won't get IN_ACCESS event because IN_MODIFY
	 * was already generated.
	 */
	SAFE_UNLINK(TEST_FILE);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, "3", 1);
	SAFE_CLOSE(fd);

	/*
	 * Possibly read the 2nd IN_MODIFY event
	 */
	errno = 0;
	len = read(fd_notify, event_buf, EVENT_BUF_LEN);
	SAFE_CLOSE(fd_notify);
	if (len < 0 && errno == EAGAIN) {
		/* Treat no event same as we treat IN_IGNORED */
		event->mask = IN_IGNORED;
	} else if (len < (int)sizeof(*event)) {
		tst_res(TFAIL | TERRNO, "Failed to read events");
		return;
	}

	if (event->mask == IN_MODIFY) {
		if (tc->expect_events > 1)
			tst_res(TPASS, "Got 2nd event as expected");
		else
			tst_res(TFAIL, "Got unexpected 2nd event");
	} else if (event->mask == IN_IGNORED) {
		if (tc->expect_events == 1)
			tst_res(TPASS, "Got no more events as expected");
		else
			tst_res(TFAIL, "Got only one event (expected %d)",
					tc->expect_events);
	} else {
		tst_res(TFAIL, "Got unexpected event 0x%x",
				event->mask);
	}
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.timeout = 10,
	.needs_tmpdir = 1,
	.cleanup = cleanup,
	.test = verify_inotify,
	.tcnt = ARRAY_SIZE(tcases),
	.tags = (const struct tst_tag[]) {
		{"linux-git", "a32e697cda27"},
		{}
	},
};

#else
	TST_TEST_TCONF("system doesn't have required inotify support");
#endif
