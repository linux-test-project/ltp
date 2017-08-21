/*
 * Copyright (c) 2013 SUSE.  All Rights Reserved.
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Started by Jan Kara <jack@suse.cz>
 *
 * DESCRIPTION
 *     Check that fanotify permission events work
 */
#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include "tst_test.h"
#include "fanotify.h"

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)

#define BUF_SIZE 256
#define TST_TOTAL 3

static char fname[BUF_SIZE];
static char buf[BUF_SIZE];
static volatile int fd_notify;

static pid_t child_pid;

static unsigned long long event_set[EVENT_MAX];
static unsigned int event_resp[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

static void generate_events(void)
{
	int fd;

	/*
	 * generate sequence of events
	 */
	if ((fd = open(fname, O_RDWR | O_CREAT, 0700)) == -1)
		exit(1);
	if (write(fd, fname, 1) == -1)
		exit(2);

	lseek(fd, 0, SEEK_SET);
	if (read(fd, buf, BUF_SIZE) != -1)
		exit(3);

	if (close(fd) == -1)
		exit(4);
}

static void child_handler(int tmp)
{
	(void)tmp;
	/*
	 * Close notification fd so that we cannot block while reading
	 * from it
	 */
	close(fd_notify);
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
		close(fd_notify);
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
	if (waitpid(-1, &child_ret, 0) < 0) {
		tst_brk(TBROK | TERRNO,
				 "waitpid(-1, &child_ret, 0) failed");
	}

	if (WIFSIGNALED(child_ret)) {
		tst_res(TFAIL, "child exited due to signal %d",
			 WTERMSIG(child_ret));
	} else if (WIFEXITED(child_ret)) {
		if (WEXITSTATUS(child_ret) == 0)
			tst_res(TPASS, "child exited correctly");
		else
			tst_res(TFAIL, "child exited with status %d",
				 WEXITSTATUS(child_ret));
	} else {
		tst_res(TFAIL, "child exited for unknown reason (status %d)",
			 child_ret);
	}
}

void test01(void)
{
	int tst_count, fd_notify_backup = -1;

	int ret, len = 0, i = 0, test_num = 0;

	if (fd_notify_backup == -1) {
		fd_notify_backup = dup(fd_notify);
		if (fd_notify_backup < 0)
			tst_brk(TBROK | TERRNO,
				 "dup(%d) failed", fd_notify);
	}
	run_child();

	tst_count = 0;

	event_set[tst_count] = FAN_OPEN_PERM;
	event_resp[tst_count++] = FAN_ALLOW;
	event_set[tst_count] = FAN_ACCESS_PERM;
	event_resp[tst_count++] = FAN_DENY;

	/* tst_count + 1 is for checking child return value */
	if (TST_TOTAL != tst_count + 1) {
		tst_brk(TBROK,
			 "TST_TOTAL and tst_count do not match");
	}
	tst_count = 0;

	/*
	 * check events
	 */
	while (test_num < TST_TOTAL && fd_notify != -1) {
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
		if (!(event->mask & event_set[test_num])) {
			tst_res(TFAIL,
				 "get event: mask=%llx (expected %llx) "
				 "pid=%u fd=%u",
				 (unsigned long long)event->mask,
				 event_set[test_num],
				 (unsigned)event->pid, event->fd);
		} else if (event->pid != child_pid) {
			tst_res(TFAIL,
				 "get event: mask=%llx pid=%u "
				 "(expected %u) fd=%u",
				 (unsigned long long)event->mask,
				 (unsigned)event->pid,
				 (unsigned)child_pid,
				 event->fd);
		} else {
			tst_res(TPASS,
				    "get event: mask=%llx pid=%u fd=%u",
				    (unsigned long long)event->mask,
				    (unsigned)event->pid, event->fd);
		}
		/* Write response to permission event */
		if (event_set[test_num] & FAN_ALL_PERM_EVENTS) {
			struct fanotify_response resp;

			resp.fd = event->fd;
			resp.response = event_resp[test_num];
			SAFE_WRITE(1, fd_notify, &resp,
				   sizeof(resp));
		}
		event->mask &= ~event_set[test_num];
		/* No events left in current mask? Go for next event */
		if (event->mask == 0) {
			i += event->event_len;
			close(event->fd);
		}
		test_num++;
	}
	for (; test_num < TST_TOTAL - 1; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%llx",
			 event_set[test_num]);

	}
	check_child();
	/* We got SIGCHLD while running, resetup fd_notify */
	if (fd_notify == -1) {
		fd_notify = fd_notify_backup;
		fd_notify_backup = -1;
	}
}

static void setup(void)
{
	int fd;

	sprintf(fname, "fname_%d", getpid());
	fd = SAFE_OPEN(fname, O_CREAT | O_RDWR, 0644);
	SAFE_WRITE(1, fd, fname, 1);
	SAFE_CLOSE(fd);

	fd_notify = SAFE_FANOTIFY_INIT(FAN_CLASS_CONTENT, O_RDONLY);

	if (fanotify_mark(fd_notify, FAN_MARK_ADD, FAN_ACCESS_PERM |
			    FAN_OPEN_PERM, AT_FDCWD, fname) < 0) {
		if (errno == EINVAL) {
			tst_brk(TCONF | TERRNO,
				 "CONFIG_FANOTIFY_ACCESS_PERMISSIONS not "
				 "configured in kernel?");
		} else {
			tst_brk(TBROK | TERRNO,
				 "fanotify_mark (%d, FAN_MARK_ADD, FAN_ACCESS_PERM | "
				 "FAN_OPEN_PERM, AT_FDCWD, %s) failed.", fd_notify, fname);
		}
	}

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
	.forks_child = 1,
	.needs_root = 1
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
