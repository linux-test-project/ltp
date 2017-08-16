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
 *     Check that fanotify work for a file
 */
#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
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
#define TST_TOTAL 12

static char fname[BUF_SIZE];
static char buf[BUF_SIZE];
static int fd, fd_notify;

static unsigned long long event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

void test01(void)
{
	int ret, len, i = 0, test_num = 0;

	int tst_count = 0;

	if (fanotify_mark(fd_notify, FAN_MARK_ADD, FAN_ACCESS | FAN_MODIFY |
			    FAN_CLOSE | FAN_OPEN, AT_FDCWD, fname) < 0) {
		tst_brk(TBROK | TERRNO,
		    "fanotify_mark (%d, FAN_MARK_ADD, FAN_ACCESS | "
		    "FAN_MODIFY | FAN_CLOSE | FAN_OPEN, AT_FDCWD, %s) "
		    "failed", fd_notify, fname);
	}

	/*
	 * generate sequence of events
	 */
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
	 * Get list of events so far. We get events here to avoid
	 * merging of following events with the previous ones.
	 */
	ret = SAFE_READ(0, fd_notify, event_buf, EVENT_BUF_LEN);
	len = ret;

	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
	event_set[tst_count] = FAN_OPEN;
	tst_count++;

	SAFE_WRITE(1, fd, fname, strlen(fname));
	event_set[tst_count] = FAN_MODIFY;
	tst_count++;

	SAFE_CLOSE(fd);
	event_set[tst_count] = FAN_CLOSE_WRITE;
	tst_count++;

	/*
	 * get another list of events
	 */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	/*
	 * Ignore mask testing
	 */

	/* Ignore access events */
	if (fanotify_mark(fd_notify,
			    FAN_MARK_ADD | FAN_MARK_IGNORED_MASK,
			    FAN_ACCESS, AT_FDCWD, fname) < 0) {
		tst_brk(TBROK | TERRNO,
		     "fanotify_mark (%d, FAN_MARK_ADD | "
		     "FAN_MARK_IGNORED_MASK, FAN_ACCESS, "
		     "AT_FDCWD, %s) failed", fd_notify, fname);
	}

	fd = SAFE_OPEN(fname, O_RDWR);
	event_set[tst_count] = FAN_OPEN;
	tst_count++;

	/* This event should be ignored */
	SAFE_READ(0, fd, buf, BUF_SIZE);

	/*
	 * get another list of events to verify the last one got ignored
	 */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	lseek(fd, 0, SEEK_SET);
	/* Generate modify event to clear ignore mask */
	SAFE_WRITE(1, fd, fname, 1);
	event_set[tst_count] = FAN_MODIFY;
	tst_count++;

	/*
	 * This event shouldn't be ignored because previous modification
	 * should have removed the ignore mask
	 */
	SAFE_READ(0, fd, buf, BUF_SIZE);
	event_set[tst_count] = FAN_ACCESS;
	tst_count++;

	SAFE_CLOSE(fd);
	event_set[tst_count] = FAN_CLOSE_WRITE;
	tst_count++;

	/* Read events to verify previous access was properly generated */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	/*
	 * Now ignore open & close events regardless of file
	 * modifications
	 */
	if (fanotify_mark(fd_notify,
			    FAN_MARK_ADD | FAN_MARK_IGNORED_MASK | FAN_MARK_IGNORED_SURV_MODIFY,
			    FAN_OPEN | FAN_CLOSE, AT_FDCWD, fname) < 0) {
		tst_brk(TBROK | TERRNO,
		     "fanotify_mark (%d, FAN_MARK_ADD | "
		     "FAN_MARK_IGNORED_MASK | "
		     "FAN_MARK_IGNORED_SURV_MODIFY, FAN_OPEN | "
		     "FAN_CLOSE, AT_FDCWD, %s) failed", fd_notify,
		     fname);
	}

	/* This event should be ignored */
	fd = SAFE_OPEN(fname, O_RDWR);

	SAFE_WRITE(1, fd, fname, 1);
	event_set[tst_count] = FAN_MODIFY;
	tst_count++;

	/* This event should be still ignored */
	SAFE_CLOSE(fd);

	/* This event should still be ignored */
	fd = SAFE_OPEN(fname, O_RDWR);

	/* Read events to verify open & close were ignored */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	/* Now remove open and close from ignored mask */
	if (fanotify_mark(fd_notify,
			    FAN_MARK_REMOVE | FAN_MARK_IGNORED_MASK,
			    FAN_OPEN | FAN_CLOSE, AT_FDCWD, fname) < 0) {
		tst_brk(TBROK | TERRNO,
		     "fanotify_mark (%d, FAN_MARK_REMOVE | "
		     "FAN_MARK_IGNORED_MASK, FAN_OPEN | "
		     "FAN_CLOSE, AT_FDCWD, %s) failed", fd_notify,
		     fname);
	}

	SAFE_CLOSE(fd);
	event_set[tst_count] = FAN_CLOSE_WRITE;
	tst_count++;

	/* Read events to verify close was generated */
	ret = SAFE_READ(0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	len += ret;

	if (TST_TOTAL != tst_count) {
		tst_brk(TBROK,
			 "TST_TOTAL (%d) and tst_count (%d) are not "
			 "equal", TST_TOTAL, tst_count);
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
				 "pid=%u fd=%u",
				 (unsigned long long)event->mask,
				 (unsigned)event->pid, event->fd);
		} else if (!(event->mask & event_set[test_num])) {
			tst_res(TFAIL,
				 "get event: mask=%llx (expected %llx) "
				 "pid=%u fd=%u",
				 (unsigned long long)event->mask,
				 event_set[test_num],
				 (unsigned)event->pid, event->fd);
		} else if (event->pid != getpid()) {
			tst_res(TFAIL,
				 "get event: mask=%llx pid=%u "
				 "(expected %u) fd=%u",
				 (unsigned long long)event->mask,
				 (unsigned)event->pid,
				 (unsigned)getpid(),
				 event->fd);
		} else {
			if (event->fd == -2)
				goto pass;
			ret = read(event->fd, buf, BUF_SIZE);
			if (ret != (int)strlen(fname)) {
				tst_res(TFAIL,
					 "cannot read from returned fd "
					 "of event: mask=%llx pid=%u "
					 "fd=%u ret=%d (errno=%d)",
					 (unsigned long long)event->mask,
					 (unsigned)event->pid,
					 event->fd, ret, errno);
			} else if (memcmp(buf, fname, strlen(fname))) {
				tst_res(TFAIL,
					 "wrong data read from returned fd "
					 "of event: mask=%llx pid=%u "
					 "fd=%u",
					 (unsigned long long)event->mask,
					 (unsigned)event->pid,
					 event->fd);
			} else {
pass:
				tst_res(TPASS,
				    "get event: mask=%llx pid=%u fd=%u",
				    (unsigned long long)event->mask,
				    (unsigned)event->pid, event->fd);
			}
		}
		/*
		 * We have verified the data now so close fd and
		 * invalidate it so that we don't check it again
		 * unnecessarily
		 */
		close(event->fd);
		event->fd = -2;
		event->mask &= ~event_set[test_num];
		/* No events left in current mask? Go for next event */
		if (event->mask == 0) {
			i += event->event_len;
		}
		test_num++;
	}
	for (; test_num < TST_TOTAL; test_num++) {
		tst_res(TFAIL, "didn't get event: mask=%llx",
			 event_set[test_num]);

	}
	/* Remove mark to clear FAN_MARK_IGNORED_SURV_MODIFY */
	if (fanotify_mark(fd_notify, FAN_MARK_REMOVE, FAN_ACCESS | FAN_MODIFY |
			    FAN_CLOSE | FAN_OPEN, AT_FDCWD, fname) < 0) {
		tst_brk(TBROK | TERRNO,
		    "fanotify_mark (%d, FAN_MARK_REMOVE, FAN_ACCESS | "
		    "FAN_MODIFY | FAN_CLOSE | FAN_OPEN, AT_FDCWD, %s) "
		    "failed", fd_notify, fname);
	}
}

static void setup(void)
{
	sprintf(fname, "tfile_%d", getpid());
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
	SAFE_WRITE(1, fd, fname, 1);
	/* close the file we have open */
	SAFE_CLOSE(fd);

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
