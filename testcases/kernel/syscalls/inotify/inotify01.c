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
/*
 * NAME
 *     inotify01
 *
 * DESCRIPTION
 *     Check that inotify work for a file
 *
 * ALGORITHM
 *     Execute sequence file's operation and check return events
 *
 * HISTORY
 *     01/06/2007 - Fix to compile inotify test case with kernel that does
 *     not support it. Ricardo Salveti de Araujo <rsalveti@linux.vnet.ibm.com>
 *
 */
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

#if defined(HAVE_SYS_INOTIFY_H) && defined(__NR_inotify_init)
#include <sys/inotify.h>

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct inotify_event))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * (EVENT_SIZE + 16))

void setup();
void cleanup();

char *TCID = "inotify01";	/* Test program identifier.    */
int TST_TOTAL = 7;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

#define BUF_SIZE 256
char fname[BUF_SIZE];
char buf[BUF_SIZE];
int fd, fd_notify;
int wd;

int event_set[EVENT_MAX];

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
		Tst_count = 0;
		/* reset Tst_count in case we are looping. */

		/*
		 * generate sequence of events
		 */
		if (chmod(fname, 0755) < 0) {
			tst_brkm(TBROK, cleanup,
				 "chmod(%s, 0755) Failed, errno=%d : %s",
				 fname, errno, strerror(errno));
		}
		event_set[Tst_count] = IN_ATTRIB;
		Tst_count++;

		if ((fd = open(fname, O_RDONLY)) == -1) {
			tst_brkm(TBROK, cleanup,
				 "open(%s, O_RDWR|O_CREAT,0700) Failed, errno=%d : %s",
				 fname, errno, strerror(errno));
		}
		event_set[Tst_count] = IN_OPEN;
		Tst_count++;

		if (read(fd, buf, BUF_SIZE) == -1) {
			tst_brkm(TBROK, cleanup,
				 "read(%d, buf, %d) Failed, errno=%d : %s",
				 fd, BUF_SIZE, errno, strerror(errno));
		}
		event_set[Tst_count] = IN_ACCESS;
		Tst_count++;

		if (close(fd) == -1) {
			tst_brkm(TBROK, cleanup,
				 "close(%s) Failed, errno=%d : %s",
				 fname, errno, strerror(errno));
		}
		event_set[Tst_count] = IN_CLOSE_NOWRITE;
		Tst_count++;

		if ((fd = open(fname, O_RDWR | O_CREAT, 0700)) == -1) {
			tst_brkm(TBROK, cleanup,
				 "open(%s, O_RDWR|O_CREAT,0700) Failed, errno=%d : %s",
				 fname, errno, strerror(errno));
		}
		event_set[Tst_count] = IN_OPEN;
		Tst_count++;

		if (write(fd, buf, BUF_SIZE) == -1) {
			tst_brkm(TBROK, cleanup,
				 "write(%d, %s, 1) Failed, errno=%d : %s",
				 fd, fname, errno, strerror(errno));
		}
		event_set[Tst_count] = IN_MODIFY;
		Tst_count++;

		if (close(fd) == -1) {
			tst_brkm(TBROK, cleanup,
				 "close(%s) Failed, errno=%d : %s",
				 fname, errno, strerror(errno));
		}
		event_set[Tst_count] = IN_CLOSE_WRITE;
		Tst_count++;

		if (TST_TOTAL != Tst_count) {
			tst_brkm(TBROK, cleanup,
				 "TST_TOTAL and Tst_count are not equal");
		}
		Tst_count = 0;

		/*
		 * get list on events
		 */
		int len, i = 0, test_num = 0;
		if ((len = read(fd_notify, event_buf, EVENT_BUF_LEN)) < 0) {
			tst_brkm(TBROK, cleanup,
				 "read(%d, buf, %d) Failed, errno=%d : %s",
				 fd_notify, EVENT_BUF_LEN, errno,
				 strerror(errno));

		}

		/*
		 * check events
		 */
		while (i < len) {
			struct inotify_event *event;
			event = (struct inotify_event *)&event_buf[i];
			if (test_num >= TST_TOTAL) {
				tst_resm(TFAIL,
					 "get unnecessary event: wd=%d mask=%x "
					 "cookie=%u len=%u",
					 event->wd, event->mask,
					 event->cookie, event->len);
			} else if (event_set[test_num] == event->mask) {
				tst_resm(TPASS, "get event: wd=%d mask=%x"
					 " cookie=%u len=%u",
					 event->wd, event->mask,
					 event->cookie, event->len);

			} else {
				tst_resm(TFAIL, "get event: wd=%d mask=%x "
					 "(expected %x) cookie=%u len=%u",
					 event->wd, event->mask,
					 event_set[test_num],
					 event->cookie, event->len);
			}
			test_num++;
			i += EVENT_SIZE + event->len;
		}
		for (; test_num < TST_TOTAL; test_num++) {
			tst_resm(TFAIL, "don't get event: mask=%x ",
				 event_set[test_num]);

		}

	}			/* End for TEST_LOOPING */

	/*
	 * cleanup and exit
	 */
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

	sprintf(fname, "tfile_%d", getpid());
	if ((fd = open(fname, O_RDWR | O_CREAT, 0700)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT,0700) Failed, errno=%d : %s",
			 fname, errno, strerror(errno));
	}
	if ((write(fd, fname, 1)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "write(%d, %s, 1) Failed, errno=%d : %s",
			 fd, fname, errno, strerror(errno));
	}

	/* close the file we have open */
	if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup,
			 "close(%s) Failed, errno=%d : %s",
			 fname, errno, strerror(errno));
	}
	if ((fd_notify = myinotify_init()) < 0) {
		if (errno == ENOSYS) {
			tst_resm(TCONF,
				 "inotify is not configured in this kernel.");
			tst_resm(TCONF, "Test will not run.");
			tst_exit();
		} else {
			tst_brkm(TBROK, cleanup,
				 "inotify_init () Failed, errno=%d : %s",
				 errno, strerror(errno));
		}
	}

	if ((wd = myinotify_add_watch(fd_notify, fname, IN_ALL_EVENTS)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "inotify_add_watch (%d, %s, IN_ALL_EVENTS)"
			 "Failed, errno=%d : %s",
			 fd_notify, fname, errno, strerror(errno));
	};

}				/* End setup() */

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *        completion or premature exit.
 */
void cleanup()
{
	if (myinotify_rm_watch(fd_notify, wd) < 0) {
		tst_resm(TWARN, "inotify_rm_watch (%d, %d) Failed,"
			 "errno=%d : %s",
			 fd_notify, wd, errno, strerror(errno));

	}

	if (close(fd_notify) == -1) {
		tst_resm(TWARN, "close(%d) Failed, errno=%d : %s",
			 fd_notify, errno, strerror(errno));
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

char *TCID = "inotify01";	/* Test program identifier.    */
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
