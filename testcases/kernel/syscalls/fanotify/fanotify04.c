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
 *     Check various fanotify special flags
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

char *TCID = "fanotify04";
int TST_TOTAL = 9;

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
static char sname[BUF_SIZE];
static char dir[BUF_SIZE];
static int fd_notify;

static int len;
static char event_buf[EVENT_BUF_LEN];

static char *expect_str_fail(int expect)
{
	if (expect == 0)
		return "failed";
	return "unexpectedly succeeded";
}

static char *expect_str_pass(int expect)
{
	if (expect == 0)
		return "succeeded";
	return "failed";
}

static void check_mark(char *file, unsigned long long flag, char *flagstr,
		       int expect, void (*test_event)(char *))
{
	if (fanotify_mark(fd_notify, FAN_MARK_ADD | flag, FAN_OPEN, AT_FDCWD,
			    file) != expect) {
		tst_resm(TFAIL,
		    "fanotify_mark (%d, FAN_MARK_ADD | %s, FAN_OPEN, AT_FDCWD, "
		    "'%s') %s", fd_notify, flagstr, file, expect_str_fail(expect));
	} else {
		tst_resm(TPASS,
		    "fanotify_mark (%d, FAN_MARK_ADD | %s, FAN_OPEN, AT_FDCWD, "
		    "'%s') %s", fd_notify, flagstr, file, expect_str_pass(expect));

		/* If we expected failure there's nothing to clean up */
		if (expect == -1)
			return;

		if (test_event)
			test_event(file);

		if (fanotify_mark(fd_notify, FAN_MARK_REMOVE | flag,
				    FAN_OPEN, AT_FDCWD, file) < 0) {
			tst_brkm(TBROK | TERRNO, cleanup,
			    "fanotify_mark (%d, FAN_MARK_REMOVE | %s, "
			    "FAN_OPEN, AT_FDCWD, '%s') failed",
			    fd_notify, flagstr, file);
		}
	}
}

#define CHECK_MARK(file, flag, expect, func) check_mark(file, flag, #flag, expect, func)

static void do_open(char *file, int flag, char *flagstr)
{
	int fd;

	fd = SAFE_OPEN(cleanup, file, O_RDONLY | flag);
	SAFE_CLOSE(cleanup, fd);
}

#define DO_OPEN(file, flag) do_open(file, flag, #flag)

static void open_file(char *file)
{
	DO_OPEN(file, 0);
}

static void open_dir(char *file)
{
	DO_OPEN(file, O_DIRECTORY);
}

static void verify_event(int mask)
{
	int ret;
	struct fanotify_event_metadata *event;
	struct stat st;

	/* Read the event */
	ret = SAFE_READ(cleanup, 0, fd_notify, event_buf + len,
			EVENT_BUF_LEN - len);
	event = (struct fanotify_event_metadata *)&event_buf[len];
	len += ret;

	if (event->mask != FAN_OPEN) {
		tst_resm(TFAIL, "got unexpected event %llx",
			 (unsigned long long)event->mask);
	} else if (fstat(event->fd, &st) < 0) {
		tst_resm(TFAIL, "failed to stat event->fd (%s)",
			 strerror(errno));
	} else if ((st.st_mode & S_IFMT) != mask) {
		tst_resm(TFAIL, "event->fd points to object of different type "
			 "(%o != %o)", st.st_mode & S_IFMT, mask);
	} else {
		tst_resm(TPASS, "event generated properly for type %o", mask);
	}
	close(event->fd);
}

static void do_open_test(char *file, int flag, char *flagstr, int mask)
{
	do_open(file, flag, flagstr);

	verify_event(mask);
}

#define DO_OPEN_TEST(file, flag, mask) do_open_test(file, flag, #flag, mask)

static void test_open_file(char *file)
{
	DO_OPEN_TEST(file, 0, S_IFREG);
}

static void verify_no_event(void)
{
	int ret;

	ret = read(fd_notify, event_buf + len, EVENT_BUF_LEN - len);
	if (ret != -1) {
		struct fanotify_event_metadata *event;

		event = (struct fanotify_event_metadata *)&event_buf[len];
		tst_resm(TFAIL, "seen unexpected event (mask %llx)",
			 (unsigned long long)event->mask);
		/* Cleanup fd from the event */
		close(event->fd);
	} else if (errno != EAGAIN) {
		tst_resm(TFAIL | TERRNO, "read(%d, buf, %zu) failed", fd_notify,
			 EVENT_BUF_LEN);
	} else {
		tst_resm(TPASS, "No event as expected");
	}
}

static void test_open_symlink(char *file)
{
	/* Since mark is on a symlink, no event should be generated by opening a file */
	DO_OPEN(file, 0);
	verify_no_event();
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Check ONLYDIR on a directory */
		CHECK_MARK(".", FAN_MARK_ONLYDIR, 0, NULL);

		/* Check ONLYDIR without a directory */
		CHECK_MARK(fname, FAN_MARK_ONLYDIR, -1, NULL);

		/* Check DONT_FOLLOW for a symlink */
		CHECK_MARK(sname, FAN_MARK_DONT_FOLLOW, 0, test_open_symlink);

		/* Check without DONT_FOLLOW for a symlink */
		CHECK_MARK(sname, 0, 0, test_open_file);

		/* Verify FAN_MARK_FLUSH destroys all inode marks */
		if (fanotify_mark(fd_notify, FAN_MARK_ADD,
				    FAN_OPEN, AT_FDCWD, fname) < 0) {
			tst_brkm(TBROK | TERRNO, cleanup,
			    "fanotify_mark (%d, FAN_MARK_ADD, FAN_OPEN, "
			    "AT_FDCWD, '%s') failed", fd_notify, fname);
		}
		if (fanotify_mark(fd_notify, FAN_MARK_ADD,
				    FAN_OPEN | FAN_ONDIR, AT_FDCWD, dir) < 0) {
			tst_brkm(TBROK | TERRNO, cleanup,
			    "fanotify_mark (%d, FAN_MARK_ADD, FAN_OPEN | "
			    "FAN_ONDIR, AT_FDCWD, '%s') failed", fd_notify,
			    dir);
		}
		open_file(fname);
		verify_event(S_IFREG);
		open_dir(dir);
		verify_event(S_IFDIR);
		if (fanotify_mark(fd_notify, FAN_MARK_FLUSH,
				    0, AT_FDCWD, ".") < 0) {
			tst_brkm(TBROK | TERRNO, cleanup,
			    "fanotify_mark (%d, FAN_MARK_FLUSH, 0, "
			    "AT_FDCWD, '.') failed", fd_notify);
		}

		open_dir(dir);
		verify_no_event();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
	sprintf(fname, "fname_%d", getpid());
	fd = SAFE_OPEN(cleanup, fname, O_RDWR | O_CREAT, 0644);
	SAFE_CLOSE(cleanup, fd);

	sprintf(sname, "symlink_%d", getpid());
	SAFE_SYMLINK(cleanup, fname, sname);

	sprintf(dir, "dir_%d", getpid());
	SAFE_MKDIR(cleanup, dir, 0755);

	if ((fd_notify = fanotify_init(FAN_CLASS_NOTIF | FAN_NONBLOCK,
					 O_RDONLY)) < 0) {
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
