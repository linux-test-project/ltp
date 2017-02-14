/*
 * Copyright (c) 2012 Linux Test Project.  All Rights Reserved.
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
 * Ngie Cooper, April 2012
 */

/****************************************************************************
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
 *
 ****************************************************************************/

#include "config.h"

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>
#endif
#include <errno.h>
#include <string.h>
#include "test.h"
#include "linux_syscall_numbers.h"
#include "inotify.h"
#include "safe_macros.h"

char *TCID = "inotify04";
int TST_TOTAL = 4;

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

static struct tst_kern_exv kvers[] = {
	{ "RHEL5", "2.6.18-132" },
	{ NULL, NULL },
};

static void cleanup(void)
{

	if (reap_wd_dir && myinotify_rm_watch(fd_notify, wd_dir) == -1)
		tst_resm(TWARN,
			 "inotify_rm_watch(%d, %d) [1] failed", fd_notify,
			 wd_dir);

	if (reap_wd_file && myinotify_rm_watch(fd_notify, wd_file) == -1)
		tst_resm(TWARN,
			 "inotify_rm_watch(%d, %d) [2] failed", fd_notify,
			 wd_file);

	if (fd_notify > 0 && close(fd_notify))
		tst_resm(TWARN, "close(%d) [1] failed", fd_notify);

	if (wd_dir > 0 && close(wd_dir))
		tst_resm(TWARN, "close(%d) [2] failed", wd_dir);

	if (wd_file > 0 && close(wd_file))
		tst_resm(TWARN, "close(%d) [3] failed", wd_file);

	tst_rmdir();
}

static void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd_notify = myinotify_init();
	if (fd_notify == -1) {
		if (errno == ENOSYS) {
			tst_brkm(TCONF, cleanup,
				 "inotify is not configured in this kernel.");
		} else {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "inotify_init failed");
		}
	}

	SAFE_MKDIR(cleanup, TEST_DIR, 00700);

	close(SAFE_CREAT(cleanup, TEST_FILE, 00600));

	wd_dir = myinotify_add_watch(fd_notify, TEST_DIR, IN_ALL_EVENTS);
	if (wd_dir == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "inotify_add_watch(%d, \"%s\", IN_ALL_EVENTS) [1] failed",
			 fd_notify, TEST_DIR);
	}
	reap_wd_dir = 1;

	wd_file = myinotify_add_watch(fd_notify, TEST_FILE, IN_ALL_EVENTS);
	if (wd_file == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "inotify_add_watch(%d, \"%s\", IN_ALL_EVENTS) [2] failed",
			 fd_notify, TEST_FILE);
	reap_wd_file = 1;
}

int main(int argc, char **argv)
{
	int i, test_num, len;

	i = 0;
	test_num = 0;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	tst_count = 0;

	rmdir(TEST_DIR);
	event_set[tst_count].mask = IN_DELETE_SELF;
	strcpy(event_set[tst_count].name, "");
	tst_count++;
	event_set[tst_count].mask = IN_IGNORED;
	strcpy(event_set[tst_count].name, "");
	tst_count++;

	unlink(TEST_FILE);
	/*
	 * When a file is unlinked, the link count is reduced by 1, and when it
	 * hits 0 the file is removed.
	 *
	 * This isn't well documented in inotify(7), but it's intuitive if you
	 * understand how Unix works.
	 */
	if (tst_kvercmp2(2, 6, 25, kvers) >= 0) {
		event_set[tst_count].mask = IN_ATTRIB;
		strcpy(event_set[tst_count].name, "");
		tst_count++;
		TST_TOTAL++;
	}
	event_set[tst_count].mask = IN_DELETE_SELF;
	strcpy(event_set[tst_count].name, TEST_FILE);
	tst_count++;
	event_set[tst_count].mask = IN_IGNORED;
	strcpy(event_set[tst_count].name, "");
	tst_count++;

	if (tst_count != TST_TOTAL)
		tst_brkm(TBROK, cleanup,
			 "tst_count and TST_TOTAL are not equal");

	tst_count = 0;

	len = read(fd_notify, event_buf, EVENT_BUF_LEN);
	if (len == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "read failed");

	reap_wd_dir = 0;
	reap_wd_file = 0;

	while (i < len) {
		struct inotify_event *event;
		event = (struct inotify_event *)&event_buf[i];
		if (test_num >= TST_TOTAL) {
			if (tst_kvercmp(2, 6, 25) < 0
			    && event_set[TST_TOTAL - 1].mask == event->mask)
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
				 "got unnecessary event: "
				 "wd=%d mask=%x cookie=%u len=%u "
				 "name=\"%.*s\"", event->wd, event->mask,
				 event->cookie, event->len, event->len, event->name);

		} else if ((event_set[test_num].mask == event->mask)
			   &&
			   (!strncmp
			    (event_set[test_num].name, event->name,
			     event->len))) {
			tst_resm(TPASS,
				 "got event: wd=%d mask=%x "
				 "cookie=%u len=%u name=\"%.*s\"",
				 event->wd, event->mask, event->cookie,
				 event->len, event->len, event->name);

		} else {
			tst_resm(TFAIL, "got event: wd=%d mask=%x "
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

	for (; test_num < TST_TOTAL; test_num++) {
		tst_resm(TFAIL, "didn't get event: mask=%x ",
			 event_set[test_num].mask);
	}

	cleanup();
	tst_exit();
}
#else
int main(void)
{
	tst_brkm(TCONF, NULL, "system doesn't have required inotify support");
}
#endif /* defined(HAVE_SYS_INOTIFY_H) */
