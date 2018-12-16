// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 SUSE.  All Rights Reserved.
 *
 * Started by Jan Kara <jack@suse.cz>
 *
 * DESCRIPTION
 *     Check that fanotify permission events are handled properly on instance
 *     destruction.
 *
 * Kernel crashes should be fixed by:
 *  96d41019e3ac "fanotify: fix list corruption in fanotify_get_response()"
 *
 * Kernel hangs should be fixed by:
 *  05f0e38724e8 "fanotify: Release SRCU lock when waiting for userspace response"
 */
#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "fanotify.h"

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>

#define BUF_SIZE 256
static char fname[BUF_SIZE];
static char buf[BUF_SIZE];
static volatile int fd_notify;

/* Number of children we start */
#define MAX_CHILDREN 16
static pid_t child_pid[MAX_CHILDREN];

/* Number of children we don't respond to before stopping */
#define MAX_NOT_RESPONDED 4

static void generate_events(void)
{
	int fd;

	/*
	 * generate sequence of events
	 */
	if ((fd = open(fname, O_RDWR | O_CREAT, 0700)) == -1)
		exit(1);

	/* Run until killed... */
	while (1) {
		lseek(fd, 0, SEEK_SET);
		if (read(fd, buf, BUF_SIZE) == -1)
			exit(3);
	}
}

static void run_children(void)
{
	int i;

	for (i = 0; i < MAX_CHILDREN; i++) {
		child_pid[i] = SAFE_FORK();
		if (!child_pid[i]) {
			/* Child will generate events now */
			close(fd_notify);
			generate_events();
			exit(0);
		}
	}
}

static int stop_children(void)
{
	int child_ret;
	int i, ret = 0;

	for (i = 0; i < MAX_CHILDREN; i++)
		SAFE_KILL(child_pid[i], SIGKILL);

	for (i = 0; i < MAX_CHILDREN; i++) {
		SAFE_WAITPID(child_pid[i], &child_ret, 0);
		if (!WIFSIGNALED(child_ret))
			ret = 1;
	}

	return ret;
}

static int setup_instance(void)
{
	int fd;

	fd = SAFE_FANOTIFY_INIT(FAN_CLASS_CONTENT, O_RDONLY);

	if (fanotify_mark(fd, FAN_MARK_ADD, FAN_ACCESS_PERM, AT_FDCWD,
			  fname) < 0) {
		close(fd);
		if (errno == EINVAL) {
			tst_brk(TCONF | TERRNO,
				"CONFIG_FANOTIFY_ACCESS_PERMISSIONS not "
				"configured in kernel?");
		} else {
			tst_brk(TBROK | TERRNO,
				"fanotify_mark (%d, FAN_MARK_ADD, FAN_ACCESS_PERM, "
				"AT_FDCWD, %s) failed.", fd, fname);
		}
	}

	return fd;
}

static void loose_fanotify_events(void)
{
	int not_responded = 0;

	/*
	 * check events
	 */
	while (not_responded < MAX_NOT_RESPONDED) {
		struct fanotify_event_metadata event;
		struct fanotify_response resp;

		/* Get more events */
		SAFE_READ(1, fd_notify, &event, sizeof(event));

		if (event.mask != FAN_ACCESS_PERM) {
			tst_res(TFAIL,
				"got event: mask=%llx (expected %llx) "
				"pid=%u fd=%d",
				(unsigned long long)event.mask,
				(unsigned long long)FAN_ACCESS_PERM,
				(unsigned)event.pid, event.fd);
			break;
		}

		/*
		 * We respond to permission event with 95% percent
		 * probability. */
		if (random() % 100 > 5) {
			/* Write response to permission event */
			resp.fd = event.fd;
			resp.response = FAN_ALLOW;
			SAFE_WRITE(1, fd_notify, &resp, sizeof(resp));
		} else {
			not_responded++;
		}
		SAFE_CLOSE(event.fd);
	}
}

static void test_fanotify(void)
{
	int newfd;
	int ret;

	fd_notify = setup_instance();
	run_children();
	loose_fanotify_events();

	/*
	 * Create and destroy another instance. This may hang if
	 * unanswered fanotify events block notification subsystem.
	 */
	newfd = setup_instance();
	if (close(newfd)) {
		tst_brk(TBROK | TERRNO, "close(%d) failed", newfd);
	}

	tst_res(TPASS, "second instance destroyed successfully");

	/*
	 * Now destroy the fanotify instance while there are permission
	 * events at various stages of processing. This may provoke
	 * kernel hangs or crashes.
	 */
	SAFE_CLOSE(fd_notify);

	ret = stop_children();
	if (ret)
		tst_res(TFAIL, "child exited for unexpected reason");
	else
		tst_res(TPASS, "all children exited successfully");
}

static void setup(void)
{
	sprintf(fname, "fname_%d", getpid());
	SAFE_FILE_PRINTF(fname, "%s", fname);
}

static void cleanup(void)
{
	if (fd_notify > 0)
		SAFE_CLOSE(fd_notify);
}

static struct tst_test test = {
	.test_all = test_fanotify,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.needs_root = 1,
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
