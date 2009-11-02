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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Started by Andrew Vagin <avagin@sw.ru>
 *
 */
/****************************************************************************
 * NAME
 *     inotify02
 *
 * DESCRIPTION
 *     Check that inotify work for a directory
 *
 * ALGORITHM
 *     Execute sequence file's operation and check return events
 *
 * HISTORY
 *     01/06/2007 - Fix to compile inotify test case with kernel that does
 *     not support it. Ricardo Salveti de Araujo <rsalveti@linux.vnet.ibm.com>
 *
 *     03/27/2008 - Fix the test failure due to event coalescence. Also add
 *     test for this event coalescence. Li Zefan <lizf@cn.fujitsu.com>
 *
 * ***************************************************************************/

#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include "test.h"
#include "usctest.h"

#if defined(__NR_inotify_init) && defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>

#ifndef IN_MOVE_SELF
#define IN_MOVE_SELF            0x00000800
#endif

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct inotify_event))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * (EVENT_SIZE + 16))

void setup();
void cleanup();

char *TCID = "inotify02";	/* Test program identifier. */
int TST_TOTAL = 9;		/* Total number of test cases.*/
extern int Tst_count;		/* Test Case counter for tst_* routines */

#define BUF_SIZE 256
char fname1[BUF_SIZE], fname2[BUF_SIZE], fname3[BUF_SIZE];
char buf[BUF_SIZE];
int fd, fd_notify;
int wd;

struct event_t {
	char name[BUF_SIZE];
	int mask;
	int len;
};
#define FILE_NAME1 "test_file1"
#define FILE_NAME2 "test_file2"

struct event_t event_set[EVENT_MAX];

char event_buf[EVENT_BUF_LEN];

static long myinotify_init()
{
	return syscall(__NR_inotify_init);
}

static long myinotify_add_watch(int fd, const char *pathname, int mask)
{
	return syscall(__NR_inotify_add_watch, fd, pathname, mask);
}

static long myinotify_rm_watch(int fd, int wd)
{
	return syscall(__NR_inotify_rm_watch, fd, wd);
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check looping state if -c option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * generate sequence of events
		 */
		if (chmod(".", 0755) < 0) {
			tst_brkm(TBROK, cleanup,
				 "chmod(\".\", 0755) Failed, errno=%d : %s",
				 errno, strerror(errno));
		}
		event_set[Tst_count].mask = IN_ISDIR | IN_ATTRIB;
		strcpy(event_set[Tst_count].name, "");
		Tst_count++;

		if ((fd = creat(FILE_NAME1, 0755)) == -1) {
			tst_brkm(TBROK, cleanup,
				 "creat(\"%s\", 755) Failed, errno=%d : %s",
				 FILE_NAME1, errno, strerror(errno));
		}

		event_set[Tst_count].mask = IN_CREATE;
		strcpy(event_set[Tst_count].name, FILE_NAME1);
		Tst_count++;
		event_set[Tst_count].mask = IN_OPEN;
		strcpy(event_set[Tst_count].name, FILE_NAME1);
		Tst_count++;

		if (close(fd) == -1) {
			tst_brkm(TBROK, cleanup,
				 "close(%s) Failed, errno=%d : %s", FILE_NAME1,
				 errno, strerror(errno));
		}
		event_set[Tst_count].mask = IN_CLOSE_WRITE;
		strcpy(event_set[Tst_count].name, FILE_NAME1);
		Tst_count++;

		if (rename(FILE_NAME1, FILE_NAME2) == -1) {
			tst_brkm(TBROK, cleanup,
				 "rename(%s, %s) Failed, errno=%d : %s",
				 FILE_NAME1, FILE_NAME2, errno,
				 strerror(errno));
		}
		event_set[Tst_count].mask = IN_MOVED_FROM;
		strcpy(event_set[Tst_count].name, FILE_NAME1);
		Tst_count++;
		event_set[Tst_count].mask = IN_MOVED_TO;
		strcpy(event_set[Tst_count].name, FILE_NAME2);
		Tst_count++;

		if (getcwd(fname1, BUF_SIZE) == NULL) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "getcwd(%p, %d) Failed", fname1,
				 BUF_SIZE);
		}

		snprintf(fname2, BUF_SIZE, "%s.rename1", fname1);
		if (rename(fname1, fname2) == -1) {
			tst_brkm(TBROK, cleanup,
				 "rename(%s, %s) Failed, errno=%d : %s", fname1,
				 fname2, errno, strerror(errno));
		}
		event_set[Tst_count].mask = IN_MOVE_SELF;
		strcpy(event_set[Tst_count].name, "");
		Tst_count++;

		if (unlink(FILE_NAME2) == -1) {
			tst_brkm(TBROK, cleanup,
				 "unlink(%s) Failed, errno=%d : %s", FILE_NAME2,
				 errno, strerror(errno));
		}
		event_set[Tst_count].mask = IN_DELETE;
		strcpy(event_set[Tst_count].name, FILE_NAME2);
		Tst_count++;

		/*
		 * test that duplicate events will be coalesced into
		 * a single event. This test case should be last, that
		 * we can correct determine kernel bug which exist before
		 * 2.6.25. See comment below.
		 */
		snprintf(fname3, BUF_SIZE, "%s.rename2", fname1);
		if (rename(fname2, fname3) == -1) {
			tst_brkm(TBROK, cleanup,
				 "rename(%s, %s) Failed, errno=%d : %s", fname2,
				 fname3, errno, strerror(errno));
		}

		if (rename(fname3, fname1) == -1) {
			tst_brkm(TBROK, cleanup,
				 "rename(%s, %s) Failed, errno=%d : %s", fname3,
				 fname1, errno, strerror(errno));
		}
		event_set[Tst_count].mask = IN_MOVE_SELF;
		strcpy(event_set[Tst_count].name, "");
		Tst_count++;

		if (Tst_count != TST_TOTAL) {
			tst_brkm(TBROK, cleanup,
				 "Tst_count and TST_TOTAL are not equal");
		}

		Tst_count = 0;

		int len, i = 0, test_num = 0;
		if ((len = read(fd_notify, event_buf, EVENT_BUF_LEN)) < 0) {
			tst_brkm(TBROK, cleanup,
				 "read(%d, buf, %d) Failed, errno=%d : %s",
				 fd_notify, EVENT_BUF_LEN, errno,
				 strerror(errno));

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
					 "name=\"%s\"", event->wd, event->mask,
					 event->cookie, event->len,
					 event->name);

			} else if ((event_set[test_num].mask == event->mask)
				   &&
				   (!strncmp
				    (event_set[test_num].name, event->name,
				     event->len))) {
				tst_resm(TPASS,
					 "get event: wd=%d mask=%x"
					 " cookie=%u len=%u name=\"%s\"",
					 event->wd, event->mask, event->cookie,
					 event->len, event->name);

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
			tst_resm(TFAIL, "don't get event: mask=%x ",
				 event_set[test_num]);
		}
	}			/* End for TEST_LOOPING */

	/* cleanup and exit */
	cleanup();

	return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	if ((fd_notify = myinotify_init()) < 0) {
		if (errno == ENOSYS) {
			tst_resm(TCONF,
				 "inotify is not configured in this kernel.");
			tst_resm(TCONF, "Test will not run.");
			tst_exit();
		} else {
			tst_brkm(TBROK, cleanup,
				 "inotify_init () Failed, errno=%d : %s", errno,
				 strerror(errno));
		}
	}

	if ((wd = myinotify_add_watch(fd_notify, ".", IN_ALL_EVENTS)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "inotify_add_watch (%d, \".\", IN_ALL_EVENTS)"
			 "Failed, errno=%d : %s", fd_notify, errno,
			 strerror(errno));
	};

}				/* End setup() */

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *        completion or premature exit.
 */
void cleanup()
{
	if (myinotify_rm_watch(fd_notify, wd) < 0) {
		tst_resm(TWARN,
			 "inotify_rm_watch (%d, %d) Failed," "errno=%d : %s",
			 fd_notify, wd, errno, strerror(errno));

	}

	if (close(fd_notify) == -1) {
		tst_resm(TWARN, "close(%d) Failed, errno=%d : %s", fd_notify,
			 errno, strerror(errno));
	}

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */

#else

char *TCID = "inotify02";	/* Test program identifier.    */
int TST_TOTAL = 0;		/* Total number of test cases. */

int main()
{
#ifndef __NR_inotify_init
	tst_resm(TCONF, "This test needs a kernel that has inotify syscall.");
	tst_resm(TCONF,
		 "Inotify syscall can be found at kernel 2.6.13 or higher.");
	return 0;
#endif
#ifndef HAVE_SYS_INOTIFY_H
	tst_resm(TBROK, "can't find header sys/inotify.h");
	return 1;
#endif
	return 0;
}

#endif
