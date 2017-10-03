/*
 * Copyright (c) 2007 SWSoft.  All Rights Reserved.
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
 * Started by Andrew Vagin <avagin@sw.ru>
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
#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"
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

static void setup(void);
static void cleanup(void);

char *TCID = "inotify02";
int TST_TOTAL = 9;

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

int main(int ac, char **av)
{
	int lc;
	unsigned int stored_cookie = UINT_MAX;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * generate sequence of events
		 */
		SAFE_CHMOD(cleanup, ".", 0755);
		event_set[tst_count].mask = IN_ISDIR | IN_ATTRIB;
		strcpy(event_set[tst_count].name, "");
		tst_count++;

		if ((fd = creat(FILE_NAME1, 0755)) == -1) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "creat(\"%s\", 755) failed", FILE_NAME1);
		}

		event_set[tst_count].mask = IN_CREATE;
		strcpy(event_set[tst_count].name, FILE_NAME1);
		tst_count++;
		event_set[tst_count].mask = IN_OPEN;
		strcpy(event_set[tst_count].name, FILE_NAME1);
		tst_count++;

		SAFE_CLOSE(cleanup, fd);
		event_set[tst_count].mask = IN_CLOSE_WRITE;
		strcpy(event_set[tst_count].name, FILE_NAME1);
		tst_count++;

		SAFE_RENAME(cleanup, FILE_NAME1, FILE_NAME2);
		event_set[tst_count].mask = IN_MOVED_FROM;
		strcpy(event_set[tst_count].name, FILE_NAME1);
		tst_count++;
		event_set[tst_count].mask = IN_MOVED_TO;
		strcpy(event_set[tst_count].name, FILE_NAME2);
		tst_count++;

		if (getcwd(fname1, BUF_SIZE) == NULL) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "getcwd(%p, %d) failed", fname1, BUF_SIZE);
		}

		snprintf(fname2, BUF_SIZE, "%s.rename1", fname1);
		SAFE_RENAME(cleanup, fname1, fname2);
		event_set[tst_count].mask = IN_MOVE_SELF;
		strcpy(event_set[tst_count].name, "");
		tst_count++;

		SAFE_UNLINK(cleanup, FILE_NAME2);
		event_set[tst_count].mask = IN_DELETE;
		strcpy(event_set[tst_count].name, FILE_NAME2);
		tst_count++;

		/*
		 * test that duplicate events will be coalesced into
		 * a single event. This test case should be last, that
		 * we can correct determine kernel bug which exist before
		 * 2.6.25. See comment below.
		 */
		snprintf(fname3, BUF_SIZE, "%s.rename2", fname1);
		SAFE_RENAME(cleanup, fname2, fname3);

		SAFE_RENAME(cleanup, fname3, fname1);
		event_set[tst_count].mask = IN_MOVE_SELF;
		strcpy(event_set[tst_count].name, "");
		tst_count++;

		if (tst_count != TST_TOTAL) {
			tst_brkm(TBROK, cleanup,
				 "tst_count and TST_TOTAL are not equal");
		}

		tst_count = 0;

		int len, i = 0, test_num = 0;
		if ((len = read(fd_notify, event_buf, EVENT_BUF_LEN)) == -1) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "read(%d, buf, %zu) failed",
				 fd_notify, EVENT_BUF_LEN);

		}

		while (i < len) {
			struct inotify_event *event;
			event = (struct inotify_event *)&event_buf[i];
			if (test_num >= TST_TOTAL) {
				if (tst_kvercmp(2, 6, 25) < 0
				    && event_set[TST_TOTAL - 1].mask ==
				    event->mask)
					tst_resm(TWARN,
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
				tst_resm(TFAIL,
					 "get unnecessary event: "
					 "wd=%d mask=%x cookie=%u len=%u"
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
					tst_resm(TPASS,
						 "get event: wd=%d mask=%x "
						 "cookie=%u len=%u name=\"%.*s\"",
						 event->wd, event->mask,
						 event->cookie, event->len,
						 event->len, event->name);
				} else {
					tst_resm(TFAIL,
						 "get event: wd=%d mask=%x "
						 "cookie=%u (wrong) len=%u "
						 "name=\"%s\"",
						 event->wd, event->mask,
						 event->cookie, event->len,
						 event->name);
				}
			} else {
				tst_resm(TFAIL, "get event: wd=%d mask=%x "
					 "(expected %x) cookie=%u len=%u "
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

		for (; test_num < TST_TOTAL; test_num++) {
			tst_resm(TFAIL, "didn't get event: mask=%x ",
				 event_set[test_num].mask);
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

	if ((fd_notify = myinotify_init()) < 0) {
		if (errno == ENOSYS) {
			tst_brkm(TCONF, cleanup,
				 "inotify is not configured in this kernel.");
		} else {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "inotify_init () failed");
		}
	}

	if ((wd = myinotify_add_watch(fd_notify, ".", IN_ALL_EVENTS)) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "inotify_add_watch (%d, \".\", IN_ALL_EVENTS) failed",
			 fd_notify);
		reap_wd = 1;
	};

}

static void cleanup(void)
{
	if (reap_wd && myinotify_rm_watch(fd_notify, wd) < 0) {
		tst_resm(TWARN,
			 "inotify_rm_watch (%d, %d) failed,", fd_notify, wd);

	}

	if (fd_notify > 0 && close(fd_notify))
		tst_resm(TWARN, "close(%d) failed", fd_notify);

	tst_rmdir();
}

#else

char *TCID = "inotify02";
int TST_TOTAL = 0;

int main(void)
{
	tst_brkm(TCONF, NULL, "system doesn't have required inotify support");
}

#endif
