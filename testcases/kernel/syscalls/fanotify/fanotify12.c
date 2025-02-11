// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Matthew Bobrowski. All Rights Reserved.
 *
 * Started by Matthew Bobrowski <mbobrowski@mbobrowski.org>
 */

/*\
 * Validate that the newly introduced FAN_OPEN_EXEC mask functions as expected.
 * The idea is to generate a sequence of open related actions to ensure that
 * the correct event flags are being set depending on what event mask was
 * requested when the object was marked.
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tst_test.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define EVENT_MAX 1024
#define EVENT_SIZE (sizeof(struct fanotify_event_metadata))
#define EVENT_BUF_LEN (EVENT_MAX * EVENT_SIZE)
#define EVENT_SET_BUF 32

#define BUF_SIZE 256
#define TEST_APP "fanotify_child"

static pid_t child_pid;
static char fname[BUF_SIZE];
static volatile int fd_notify;
static volatile int complete;
static char event_buf[EVENT_BUF_LEN];
static int exec_events_unsupported;

static struct test_case_t {
	const char *tname;
	struct fanotify_mark_type mark;
	unsigned long long mask;
	unsigned long long ignore_mask;
	int event_count;
	unsigned long long event_set[EVENT_SET_BUF];
} test_cases[] = {
	{
		"inode mark, FAN_OPEN events",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_OPEN,
		0,
		2,
		{FAN_OPEN, FAN_OPEN}
	},
	{
		"inode mark, FAN_OPEN_EXEC events",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_OPEN_EXEC,
		0,
		1,
		{FAN_OPEN_EXEC}
	},
	{
		"inode mark, FAN_OPEN | FAN_OPEN_EXEC events",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_OPEN | FAN_OPEN_EXEC,
		0,
		2,
		{FAN_OPEN, FAN_OPEN | FAN_OPEN_EXEC}
	},
	{
		"inode mark, FAN_OPEN events, ignore FAN_OPEN_EXEC",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_OPEN,
		FAN_OPEN_EXEC,
		2,
		{FAN_OPEN, FAN_OPEN}
	},
	{
		"inode mark, FAN_OPEN_EXEC events, ignore FAN_OPEN",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_OPEN_EXEC,
		FAN_OPEN,
		1,
		{FAN_OPEN_EXEC}
	},
	{
		"inode mark, FAN_OPEN | FAN_OPEN_EXEC events, ignore "
		"FAN_OPEN_EXEC",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_OPEN | FAN_OPEN_EXEC,
		FAN_OPEN_EXEC,
		2,
		{FAN_OPEN, FAN_OPEN}
	}
};

static int generate_events(void)
{
	int fd, status;

	child_pid = SAFE_FORK();

	/*
	 * Generate a sequence of events
	 */
	if (child_pid == 0) {
		SAFE_CLOSE(fd_notify);

		fd = SAFE_OPEN(fname, O_RDONLY);

		if (fd > 0)
			SAFE_CLOSE(fd);

		SAFE_EXECL(TEST_APP, TEST_APP, NULL);
		exit(1);
	}

	SAFE_WAITPID(child_pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return 1;
	return 0;
}

static int setup_mark(unsigned int n)
{
	unsigned int i = 0;
	struct test_case_t *tc = &test_cases[n];
	struct fanotify_mark_type *mark = &tc->mark;
	const char *const files[] = {fname, TEST_APP};

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (exec_events_unsupported && ((tc->mask & FAN_OPEN_EXEC) ||
					tc->ignore_mask & FAN_OPEN_EXEC)) {
		tst_res(TCONF, "FAN_OPEN_EXEC not supported in kernel?");
		return -1;
	}

	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF, O_RDONLY);

	for (; i < ARRAY_SIZE(files); i++) {
		/* Setup normal mark on object */
		SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD | mark->flag,
					tc->mask, AT_FDCWD, files[i]);

		/* Setup ignore mark on object */
		if (tc->ignore_mask) {
			SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD | mark->flag
						| FAN_MARK_IGNORED_MASK,
						tc->ignore_mask, AT_FDCWD,
						files[i]);
		}
	}

	return 0;
}

static void do_test(unsigned int n)
{
	int len = 0, event_num = 0;
	struct test_case_t *tc = &test_cases[n];
	struct fanotify_event_metadata *event;

	/* Place a mark on the object */
	if (setup_mark(n) != 0)
		goto cleanup;

	/* Generate events in child process */
	if (!generate_events())
		goto cleanup;

	/* Read available events into buffer */
	len = SAFE_READ(0, fd_notify, event_buf, EVENT_BUF_LEN);

	event = (struct fanotify_event_metadata *) event_buf;

	/* Process events */
	while (FAN_EVENT_OK(event, len) && event_num < tc->event_count) {
		if (event->mask != *(tc->event_set + event_num)) {
			tst_res(TFAIL,
				"Received event: mask=%llx (expected %llx, "
				"pid=%u, fd=%d",
				(unsigned long long) event->mask,
				*(tc->event_set + event_num),
				(unsigned int) event->pid,
				event->fd);
		} else if (event->pid != child_pid) {
			tst_res(TFAIL,
				"Received event: mask=%llx, pid=%u (expected "
				"%u), fd=%d",
				(unsigned long long) event->mask,
				(unsigned int) event->pid,
				(unsigned int) child_pid,
				event->fd);
		} else {
			tst_res(TPASS,
				"Received event: mask=%llx, pid=%u, fd=%d",
				(unsigned long long) event->mask,
				(unsigned int) event->pid,
				event->fd);
		}

		if (event->fd != FAN_NOFD)
			SAFE_CLOSE(event->fd);

		event_num++;
		event = FAN_EVENT_NEXT(event, len);
	}

cleanup:
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static void do_setup(void)
{
	sprintf(fname, "fname_%d", getpid());
	SAFE_FILE_PRINTF(fname, "1");
	exec_events_unsupported = fanotify_flags_supported_on_fs(FAN_CLASS_NOTIF,
					0, FAN_OPEN_EXEC, fname);
}

static void do_cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.setup = do_setup,
	.test = do_test,
	.tcnt = ARRAY_SIZE(test_cases),
	.cleanup = do_cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.resource_files = (const char *const []) {
		TEST_APP,
		NULL
	}
};
#else
	TST_TEST_TCONF("System does not contain required fanotify support");
#endif
