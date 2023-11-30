// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2013 SUSE.  All Rights Reserved.
 *
 * Started by Jan Kara <jack@suse.cz>
 */

/*\
 * [Description]
 * Check that fanotify permission events work.
 */

#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include "tst_test.h"

#ifdef HAVE_SYS_FANOTIFY_H
# include "fanotify.h"

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof(struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)
/* Size large enough to hold a reasonable amount of expected event objects */
#define EVENT_SET_MAX 16

#define BUF_SIZE 256
#define TST_TOTAL 3
#define TEST_APP "fanotify_child"
#define MOUNT_PATH "fs_mnt"
#define FILE_EXEC_PATH MOUNT_PATH"/"TEST_APP

static char fname[BUF_SIZE];
static char buf[BUF_SIZE];
static volatile int fd_notify;

static pid_t child_pid;

static char event_buf[EVENT_BUF_LEN];
static int exec_events_unsupported;
static int filesystem_mark_unsupported;

struct event {
	unsigned long long mask;
	unsigned int response;
};

static struct tcase {
	const char *tname;
	struct fanotify_mark_type mark;
	unsigned long long mask;
	int event_count;
	struct event event_set[EVENT_SET_MAX];
} tcases[] = {
	{
		"inode mark, FAN_OPEN_PERM | FAN_ACCESS_PERM events",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_OPEN_PERM | FAN_ACCESS_PERM, 3,
		{
			{FAN_OPEN_PERM, FAN_ALLOW},
			{FAN_ACCESS_PERM, FAN_DENY},
			{FAN_OPEN_PERM, FAN_DENY}
		}
	},
	{
		"inode mark, FAN_ACCESS_PERM | FAN_OPEN_EXEC_PERM events",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_ACCESS_PERM | FAN_OPEN_EXEC_PERM, 2,
		{
			{FAN_ACCESS_PERM, FAN_DENY},
			{FAN_OPEN_EXEC_PERM, FAN_DENY}
		}
	},
	{
		"mount mark, FAN_OPEN_PERM | FAN_ACCESS_PERM events",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN_PERM | FAN_ACCESS_PERM, 3,
		{
			{FAN_OPEN_PERM, FAN_ALLOW},
			{FAN_ACCESS_PERM, FAN_DENY},
			{FAN_OPEN_PERM, FAN_DENY}
		}
	},
	{
		"mount mark, FAN_ACCESS_PERM | FAN_OPEN_EXEC_PERM events",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_ACCESS_PERM | FAN_OPEN_EXEC_PERM, 2,
		{
			{FAN_ACCESS_PERM, FAN_DENY},
			{FAN_OPEN_EXEC_PERM, FAN_DENY}
		}
	},
	{
		"filesystem mark, FAN_OPEN_PERM | FAN_ACCESS_PERM events",
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_OPEN_PERM | FAN_ACCESS_PERM, 3,
		{
			{FAN_OPEN_PERM, FAN_ALLOW},
			{FAN_ACCESS_PERM, FAN_DENY},
			{FAN_OPEN_PERM, FAN_DENY}
		}
	},
	{
		"filesystem mark, FAN_ACCESS_PERM | FAN_OPEN_EXEC_PERM events",
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_ACCESS_PERM | FAN_OPEN_EXEC_PERM, 2,
		{
			{FAN_ACCESS_PERM, FAN_DENY},
			{FAN_OPEN_EXEC_PERM, FAN_DENY}
		}
	},
};

static void generate_events(void)
{
	int fd;
	char *const argv[] = {FILE_EXEC_PATH, NULL};

	/*
	 * Generate sequence of events
	 */
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);

	SAFE_WRITE(SAFE_WRITE_ANY, fd, fname, 1);
	SAFE_LSEEK(fd, 0, SEEK_SET);

	if (read(fd, buf, BUF_SIZE) != -1)
		exit(3);

	SAFE_CLOSE(fd);

	if (execve(FILE_EXEC_PATH, argv, environ) != -1)
		exit(5);
}

static void child_handler(int tmp)
{
	(void)tmp;
	/*
	 * Close notification fd so that we cannot block while reading
	 * from it
	 */
	SAFE_CLOSE(fd_notify);
	fd_notify = -1;
}

static void run_child(void)
{
	struct sigaction child_action;

	child_action.sa_handler = child_handler;
	sigemptyset(&child_action.sa_mask);
	child_action.sa_flags = SA_NOCLDSTOP;

	if (sigaction(SIGCHLD, &child_action, NULL) < 0) {
		tst_brk(TBROK | TERRNO,
			"sigaction(SIGCHLD, &child_action, NULL) failed");
	}

	child_pid = SAFE_FORK();

	if (child_pid == 0) {
		/* Child will generate events now */
		SAFE_CLOSE(fd_notify);
		generate_events();
		exit(0);
	}
}

static void check_child(void)
{
	struct sigaction child_action;
	int child_ret;

	child_action.sa_handler = SIG_IGN;
	sigemptyset(&child_action.sa_mask);
	child_action.sa_flags = SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &child_action, NULL) < 0) {
		tst_brk(TBROK | TERRNO,
			"sigaction(SIGCHLD, &child_action, NULL) failed");
	}
	SAFE_WAITPID(-1, &child_ret, 0);

	if (WIFEXITED(child_ret) && WEXITSTATUS(child_ret) == 0)
		tst_res(TPASS, "child exited correctly");
	else
		tst_res(TFAIL, "child %s", tst_strstatus(child_ret));
}

static int setup_mark(unsigned int n)
{
	unsigned int i = 0;
	struct tcase *tc = &tcases[n];
	struct fanotify_mark_type *mark = &tc->mark;
	char *const files[] = {fname, FILE_EXEC_PATH};

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (exec_events_unsupported && tc->mask & FAN_OPEN_EXEC_PERM) {
		tst_res(TCONF, "FAN_OPEN_EXEC_PERM not supported in kernel?");
		return -1;
	}

	if (filesystem_mark_unsupported && mark->flag == FAN_MARK_FILESYSTEM) {
		tst_res(TCONF, "FAN_MARK_FILESYSTEM not supported in kernel?");
		return -1;
	}

	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_CONTENT, O_RDONLY);

	for (; i < ARRAY_SIZE(files); i++) {
		SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD | mark->flag,
				  tc->mask, AT_FDCWD, files[i]);
	}

	return 0;
}

static void test_fanotify(unsigned int n)
{
	int ret, len = 0, i = 0, test_num = 0;
	struct tcase *tc = &tcases[n];
	struct event *event_set = tc->event_set;

	if (setup_mark(n) != 0)
		return;

	run_child();

	/*
	 * Process events
	 *
	 * tc->count + 1 is to accommodate for checking the child process
	 * return value
	 */
	while (test_num < tc->event_count + 1 && fd_notify != -1) {
		struct fanotify_event_metadata *event;

		if (i == len) {
			/* Get more events */
			ret = read(fd_notify, event_buf + len,
				   EVENT_BUF_LEN - len);
			if (fd_notify == -1)
				break;
			if (ret < 0) {
				tst_brk(TBROK,
					"read(%d, buf, %zu) failed",
					fd_notify, EVENT_BUF_LEN);
			}
			len += ret;
		}

		event = (struct fanotify_event_metadata *)&event_buf[i];
		/* Permission events cannot be merged, so the event mask
		 * reported should exactly match the event mask within the
		 * event set.
		 */
		if (event->mask != event_set[test_num].mask) {
			tst_res(TFAIL,
				"got event: mask=%llx (expected %llx) "
				"pid=%u fd=%d",
				(unsigned long long)event->mask,
				event_set[test_num].mask,
				(unsigned int)event->pid, event->fd);
		} else if (event->pid != child_pid) {
			tst_res(TFAIL,
				"got event: mask=%llx pid=%u "
				"(expected %u) fd=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid,
				(unsigned int)child_pid,
				event->fd);
		} else {
			tst_res(TPASS,
				"got event: mask=%llx pid=%u fd=%d",
				(unsigned long long)event->mask,
				(unsigned int)event->pid, event->fd);
		}

		/* Write response to the permission event */
		if (event_set[test_num].mask & LTP_ALL_PERM_EVENTS) {
			struct fanotify_response resp;

			resp.fd = event->fd;
			resp.response = event_set[test_num].response;
			SAFE_WRITE(SAFE_WRITE_ALL, fd_notify, &resp, sizeof(resp));
		}

		i += event->event_len;

		if (event->fd != FAN_NOFD)
			SAFE_CLOSE(event->fd);

		test_num++;
	}

	for (; test_num < tc->event_count; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%llx",
			event_set[test_num].mask);

	}

	check_child();

	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static void setup(void)
{
	sprintf(fname, MOUNT_PATH"/fname_%d", getpid());
	SAFE_FILE_PRINTF(fname, "1");

	require_fanotify_access_permissions_supported_on_fs(fname);
	filesystem_mark_unsupported = fanotify_mark_supported_on_fs(FAN_MARK_FILESYSTEM, fname);
	exec_events_unsupported = fanotify_flags_supported_on_fs(FAN_CLASS_CONTENT,
					0, FAN_OPEN_EXEC_PERM, fname);

	SAFE_CP(TEST_APP, FILE_EXEC_PATH);
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static const char *const resource_files[] = {
	TEST_APP,
	NULL
};

static struct tst_test test = {
	.test = test_fanotify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MOUNT_PATH,
	.resource_files = resource_files
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
