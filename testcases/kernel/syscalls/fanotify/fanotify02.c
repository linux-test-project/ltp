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
 *     Check that fanotify work for children of a directory
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
#include "test.h"
#include "linux_syscall_numbers.h"
#include "fanotify.h"
#include "safe_macros.h"

char *TCID = "fanotify02";
int TST_TOTAL = 8;

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)

static void setup(void);
static void cleanup(void);

#define BUF_SIZE 256
static char fname[BUF_SIZE];
static char buf[BUF_SIZE];
static int fd, fd_notify;

static unsigned long long event_set[EVENT_MAX];

static char event_buf[EVENT_BUF_LEN];

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int ret, len, i = 0, test_num = 0;

		tst_count = 0;

		if (fanotify_mark(fd_notify, FAN_MARK_ADD, FAN_ACCESS |
				    FAN_MODIFY | FAN_CLOSE | FAN_OPEN |
				    FAN_EVENT_ON_CHILD | FAN_ONDIR, AT_FDCWD,
				  ".") < 0) {
			tst_brkm(TBROK | TERRNO, cleanup,
			    "fanotify_mark (%d, FAN_MARK_ADD, FAN_ACCESS | "
			    "FAN_MODIFY | FAN_CLOSE | FAN_OPEN | "
			    "FAN_EVENT_ON_CHILD | FAN_ONDIR, AT_FDCWD, '.') "
			    "failed", fd_notify);
		}

		/*
		 * generate sequence of events
		 */
		fd = SAFE_OPEN(cleanup, fname, O_RDWR | O_CREAT, 0700);
		event_set[tst_count] = FAN_OPEN;
		tst_count++;

		SAFE_WRITE(cleanup, 1, fd, fname, strlen(fname));
		event_set[tst_count] = FAN_MODIFY;
		tst_count++;

		SAFE_CLOSE(cleanup, fd);
		event_set[tst_count] = FAN_CLOSE_WRITE;
		tst_count++;

		/*
		 * Get list of events so far. We get events here to avoid
		 * merging of following events with the previous ones.
		 */
		ret = SAFE_READ(cleanup, 0, fd_notify, event_buf,
				EVENT_BUF_LEN);
		len = ret;

		fd = SAFE_OPEN(cleanup, fname, O_RDONLY);
		event_set[tst_count] = FAN_OPEN;
		tst_count++;

		SAFE_READ(cleanup, 0, fd, buf, BUF_SIZE);
		event_set[tst_count] = FAN_ACCESS;
		tst_count++;

		SAFE_CLOSE(cleanup, fd);
		event_set[tst_count] = FAN_CLOSE_NOWRITE;
		tst_count++;

		/*
		 * get next events
		 */
		ret = SAFE_READ(cleanup, 0, fd_notify, event_buf + len,
				EVENT_BUF_LEN - len);
		len += ret;

		/*
		 * now remove child mark
		 */
		if (fanotify_mark(fd_notify, FAN_MARK_REMOVE,
				    FAN_EVENT_ON_CHILD, AT_FDCWD, ".") < 0) {
			tst_brkm(TBROK | TERRNO, cleanup,
			    "fanotify_mark (%d, FAN_MARK REMOVE, "
			    "FAN_EVENT_ON_CHILD, AT_FDCWD, '.') failed",
			    fd_notify);
		}

		/*
		 * Do something to verify events didn't get generated
		 */
		fd = SAFE_OPEN(cleanup, fname, O_RDONLY);

		SAFE_CLOSE(cleanup, fd);

		fd = SAFE_OPEN(cleanup, ".", O_RDONLY | O_DIRECTORY);
		event_set[tst_count] = FAN_OPEN;
		tst_count++;

		SAFE_CLOSE(cleanup, fd);
		event_set[tst_count] = FAN_CLOSE_NOWRITE;
		tst_count++;

		/*
		 * Check events got generated only for the directory
		 */
		ret = SAFE_READ(cleanup, 0, fd_notify, event_buf + len,
				EVENT_BUF_LEN - len);
		len += ret;

		if (TST_TOTAL != tst_count) {
			tst_brkm(TBROK, cleanup,
				 "TST_TOTAL and tst_count are not equal");
		}
		tst_count = 0;

		/*
		 * check events
		 */
		while (i < len) {
			struct fanotify_event_metadata *event;

			event = (struct fanotify_event_metadata *)&event_buf[i];
			if (test_num >= TST_TOTAL) {
				tst_resm(TFAIL,
					 "get unnecessary event: mask=%llx "
					 "pid=%u fd=%u",
					 (unsigned long long)event->mask,
					 (unsigned)event->pid, event->fd);
			} else if (!(event->mask & event_set[test_num])) {
				tst_resm(TFAIL,
					 "get event: mask=%llx (expected %llx) "
					 "pid=%u fd=%u",
					 (unsigned long long)event->mask,
					 event_set[test_num],
					 (unsigned)event->pid, event->fd);
			} else if (event->pid != getpid()) {
				tst_resm(TFAIL,
					 "get event: mask=%llx pid=%u "
					 "(expected %u) fd=%u",
					 (unsigned long long)event->mask,
					 (unsigned)event->pid,
					 (unsigned)getpid(),
					 event->fd);
			} else {
				tst_resm(TPASS,
					    "get event: mask=%llx pid=%u fd=%u",
					    (unsigned long long)event->mask,
					    (unsigned)event->pid, event->fd);
			}
			event->mask &= ~event_set[test_num];
			/* No events left in current mask? Go for next event */
			if (event->mask == 0) {
				i += event->event_len;
				close(event->fd);
			}
			test_num++;
		}
		for (; test_num < TST_TOTAL; test_num++) {
			tst_resm(TFAIL, "didn't get event: mask=%llx",
				 event_set[test_num]);

		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
	sprintf(fname, "fname_%d", getpid());

	if ((fd_notify = fanotify_init(FAN_CLASS_NOTIF, O_RDONLY)) < 0) {
		if (errno == ENOSYS) {
			tst_brkm(TCONF, cleanup,
				 "fanotify is not configured in this kernel.");
		} else {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "fanotify_init failed");
		}
	}
}

static void cleanup(void)
{
	if (fd_notify > 0 && close(fd_notify))
		tst_resm(TWARN | TERRNO, "close(%d) failed", fd_notify);

	tst_rmdir();
}

#else

int main(void)
{
	tst_brkm(TCONF, NULL, "system doesn't have required fanotify support");
}

#endif
