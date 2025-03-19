// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 SUSE.  All Rights Reserved.
 * Copyright (c) 2025 CTERA Networks.  All Rights Reserved.
 *
 * Started by Jan Kara <jack@suse.cz>
 */

/*\
 * - Test fanotify pre-content events
 * - Test respond to permission/pre-content events with cutsom error code
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

struct event {
	unsigned long long mask;
	unsigned int response;
};

static struct tcase {
	const char *tname;
	struct fanotify_mark_type mark;
	unsigned long long mask;
	struct event event_set[EVENT_SET_MAX];
} tcases[] = {
	{
		"inode mark, FAN_OPEN_PERM | FAN_PRE_ACCESS events",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_OPEN_PERM | FAN_PRE_ACCESS,
		{
			{FAN_OPEN_PERM, FAN_ALLOW},
			{FAN_PRE_ACCESS, FAN_DENY},
			{FAN_PRE_ACCESS, FAN_DENY_ERRNO(EIO)},
			{FAN_OPEN_PERM, FAN_DENY_ERRNO(EBUSY)}
		}
	},
	{
		"inode mark, FAN_PRE_ACCESS | FAN_OPEN_EXEC_PERM events",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_PRE_ACCESS | FAN_OPEN_EXEC_PERM,
		{
			{FAN_PRE_ACCESS, FAN_DENY},
			{FAN_PRE_ACCESS, FAN_DENY_ERRNO(EIO)},
			{FAN_OPEN_EXEC_PERM, FAN_DENY_ERRNO(EBUSY)}
		}
	},
	{
		"mount mark, FAN_OPEN_PERM | FAN_PRE_ACCESS events",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_OPEN_PERM | FAN_PRE_ACCESS,
		{
			{FAN_OPEN_PERM, FAN_ALLOW},
			{FAN_PRE_ACCESS, FAN_DENY},
			{FAN_PRE_ACCESS, FAN_DENY_ERRNO(EIO)},
			{FAN_OPEN_PERM, FAN_DENY_ERRNO(EBUSY)}
		}
	},
	{
		"mount mark, FAN_PRE_ACCESS | FAN_OPEN_EXEC_PERM events",
		INIT_FANOTIFY_MARK_TYPE(MOUNT),
		FAN_PRE_ACCESS | FAN_OPEN_EXEC_PERM,
		{
			{FAN_PRE_ACCESS, FAN_DENY},
			{FAN_PRE_ACCESS, FAN_DENY_ERRNO(EIO)},
			{FAN_OPEN_EXEC_PERM, FAN_DENY_ERRNO(EBUSY)}
		}
	},
	{
		"filesystem mark, FAN_OPEN_PERM | FAN_PRE_ACCESS events",
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_OPEN_PERM | FAN_PRE_ACCESS,
		{
			{FAN_OPEN_PERM, FAN_ALLOW},
			{FAN_PRE_ACCESS, FAN_DENY},
			{FAN_PRE_ACCESS, FAN_DENY_ERRNO(EIO)},
			{FAN_OPEN_PERM, FAN_DENY_ERRNO(EBUSY)}
		}
	},
	{
		"filesystem mark, FAN_PRE_ACCESS | FAN_OPEN_EXEC_PERM events",
		INIT_FANOTIFY_MARK_TYPE(FILESYSTEM),
		FAN_PRE_ACCESS | FAN_OPEN_EXEC_PERM,
		{
			{FAN_PRE_ACCESS, FAN_DENY},
			{FAN_PRE_ACCESS, FAN_DENY_ERRNO(EIO)},
			{FAN_OPEN_EXEC_PERM, FAN_DENY_ERRNO(EBUSY)}
		}
	},
	{
		"parent watching children, FAN_PRE_ACCESS | FAN_OPEN_EXEC_PERM events",
		INIT_FANOTIFY_MARK_TYPE(PARENT),
		FAN_PRE_ACCESS | FAN_OPEN_EXEC_PERM | FAN_EVENT_ON_CHILD,
		{
			{FAN_PRE_ACCESS, FAN_DENY},
			{FAN_PRE_ACCESS, FAN_DENY},
			{FAN_OPEN_EXEC_PERM, FAN_DENY}
		}
	},
	{
		"parent not watching children, FAN_PRE_ACCESS | FAN_OPEN_EXEC_PERM events",
		INIT_FANOTIFY_MARK_TYPE(PARENT),
		FAN_PRE_ACCESS | FAN_OPEN_EXEC_PERM,
		{
		}
	},
	{
		"inode mark, FAN_PRE_ACCESS event allowed",
		INIT_FANOTIFY_MARK_TYPE(INODE),
		FAN_PRE_ACCESS,
		{
			/* This allows multiple FAN_PRE_ACCESS events */
			{FAN_PRE_ACCESS, FAN_ALLOW},
		}
	},
};

static int expected_errno(unsigned int response)
{
	switch (response) {
	case 0:
	case FAN_ALLOW:
		return 0;
	case FAN_DENY:
		return EPERM;
	default:
		return FAN_RESPONSE_ERRNO(response);
	}
}

static void generate_events(struct tcase *tc)
{
	int fd;
	char *const argv[] = {FILE_EXEC_PATH, NULL};
	struct event *event = tc->event_set;
	int exp_ret, exp_errno = 0;

	if (event->mask == FAN_OPEN_PERM)
		event++;

	/*
	 * Generate sequence of events
	 */
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);

	exp_errno = expected_errno(event->response);
	event++;

	exp_ret = exp_errno ? -1 : 1;
	errno = 0;
	/* FAN_PRE_ACCESS events are reported also on write */
	if (write(fd, fname, 1) != exp_ret || errno != exp_errno) {
		tst_res(TFAIL, "write() got errno %d (expected %d)", errno, exp_errno);
		exit(3);
	} else if (errno == exp_errno) {
		tst_res(TINFO, "write() got errno %d as expected", errno);
	}

	SAFE_LSEEK(fd, 0, SEEK_SET);

	exp_errno = expected_errno(event->response);
	event++;

	exp_ret = exp_errno ? -1 : 1;
	errno = 0;
	if (read(fd, buf, BUF_SIZE) != exp_ret || errno != exp_errno) {
		tst_res(TFAIL, "read() got errno %d (expected %d)", errno, exp_errno);
		exit(4);
	} else if (errno == exp_errno) {
		tst_res(TINFO, "read() got errno %d as expected", errno);
	}

	SAFE_CLOSE(fd);

	exp_errno = expected_errno(event->response);
	event++;

	/*
	 * If execve() is allowed by permission events, check if executing a
	 * file that open for write is allowed.
	 * HSM needs to be able to write to file during pre-content event, so it
	 * requires that a file being executed can be open for write, which also
	 * means that a file open for write can be executed.
	 * Therefore, ETXTBSY is to be expected when file is not being watched
	 * at all or being watched but not with pre-content events in mask.
	 */
	if (!exp_errno) {
		fd = SAFE_OPEN(FILE_EXEC_PATH, O_RDWR);
		if (!tc->event_set[0].mask)
			exp_errno = ETXTBSY;
	}

	exp_ret = exp_errno ? -1 : 0;
	errno = 0;
	if (execve(FILE_EXEC_PATH, argv, environ) != exp_ret || errno != exp_errno) {
		tst_res(TFAIL, "execve() got errno %d (expected %d)", errno, exp_errno);
		exit(5);
	} else if (errno == exp_errno) {
		tst_res(TINFO, "execve() got errno %d as expected", errno);
	}

	if (fd >= 0)
		SAFE_CLOSE(fd);
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

static void run_child(struct tcase *tc)
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
		generate_events(tc);
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

	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_PRE_CONTENT, O_RDONLY);

	if (mark->flag == FAN_MARK_PARENT) {
		SAFE_FANOTIFY_MARK(fd_notify, FAN_MARK_ADD | mark->flag,
				   tc->mask, AT_FDCWD, MOUNT_PATH);
		return 0;
	}

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

	run_child(tc);

	/*
	 * Process events
	 *
	 * even if we do not expect another event, let read() wait for child
	 * process to exit and accomodate for multiple access events
	 */
	while (test_num < EVENT_SET_MAX && fd_notify != -1) {
		struct fanotify_event_metadata *event;

		if (i == len) {
			/* Get more events */
			ret = read(fd_notify, event_buf + len,
				   EVENT_BUF_LEN - len);
			/* Received SIGCHLD */
			if (fd_notify == -1)
				break;
			if (ret < 0) {
				tst_brk(TBROK,
					"read(%d, buf, %zu) failed",
					fd_notify, EVENT_BUF_LEN);
			}
			len += ret;
		}

		/*
		 * If we got an event after the last event and the last event was
		 * allowed then assume this is another event of the same type.
		 * This is to accomodate for the fact that a single read() may
		 * generate an unknown number of access permission events if they
		 * are allowed.
		 */
		if (test_num > 0 && !event_set[test_num].mask &&
		    event_set[test_num-1].response == FAN_ALLOW)
			test_num--;

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
		if (event_set[test_num].mask &
			(LTP_ALL_PERM_EVENTS | LTP_PRE_CONTENT_EVENTS)) {
			struct fanotify_response resp;

			resp.fd = event->fd;
			resp.response = event_set[test_num].response;
			SAFE_WRITE(SAFE_WRITE_ALL, fd_notify, &resp, sizeof(resp));
			tst_res(TPASS, "response=%x fd=%d", resp.response, resp.fd);
		}

		i += event->event_len;

		if (event->fd != FAN_NOFD) {
			char c;

			/* Verify that read from event fd does not generate events */
			SAFE_READ(0, event->fd, &c, 1);
			SAFE_CLOSE(event->fd);
		}

		test_num++;
	}

	for (; event_set[test_num].mask && test_num < EVENT_SET_MAX; test_num++) {
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

	REQUIRE_FANOTIFY_EVENTS_SUPPORTED_ON_FS(FAN_CLASS_PRE_CONTENT, FAN_MARK_FILESYSTEM,
						FAN_PRE_ACCESS, fname);

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
	.timeout = 1,
	.test = test_fanotify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.mount_device = 1,
	.all_filesystems = 1,
	.mntpoint = MOUNT_PATH,
	.resource_files = resource_files
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
