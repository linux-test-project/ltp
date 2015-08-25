/*
 * Copyright (c) 2015 SUSE Linux.  All Rights Reserved.
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
 * Started by Jan Kara <jack@suse.cz>
 *
 * DESCRIPTION
 * Test for inotify mark destruction race.
 *
 * Kernels prior to 4.2 have a race when inode is being deleted while
 * inotify group watching that inode is being torn down. When the race is
 * hit, the kernel crashes or loops.
 *
 * The problem has been fixed by commit:
 *  8f2f3eb59dff "fsnotify: fix oops in fsnotify_clear_marks_by_group_flags()".
 */

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include "test.h"
#include "linux_syscall_numbers.h"
#include "inotify.h"
#include "safe_macros.h"

char *TCID = "inotify06";
int TST_TOTAL = 1;

#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>

/* Number of test loops to run the test for */
#define TEARDOWNS 400

/* Number of files to test (must be > 1) */
#define FILES 5

char names[FILES][PATH_MAX];

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	int i;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	for (i = 0; i < FILES; i++)
		sprintf(names[i], "fname_%d", i);
}

static void verify_inotify(void)
{
	int inotify_fd, fd;
	pid_t pid;
	int i, tests;

	pid = tst_fork();
	if (pid == 0) {
		while (1) {
			for (i = 0; i < FILES; i++) {
				fd = SAFE_OPEN(NULL, names[i], O_CREAT | O_RDWR, 0600);
				SAFE_CLOSE(NULL, fd);
			}
			for (i = 0; i < FILES; i++)
				SAFE_UNLINK(NULL, names[i]);
		}
	} else if (pid == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "fork() failed");

	for (tests = 0; tests < TEARDOWNS; tests++) {
		inotify_fd = ltp_syscall(__NR_inotify_init1, O_NONBLOCK);
		if (inotify_fd < 0)
			tst_brkm(TBROK | TERRNO, cleanup, "inotify_init failed");

		for (i = 0; i < FILES; i++) {
			/*
			 * Both failure and success are fine since
			 * files are being deleted in parallel - this
			 * is what provokes the race we want to test
			 * for...
			 */
			myinotify_add_watch(inotify_fd, names[i], IN_MODIFY);
		}
		SAFE_CLOSE(cleanup, inotify_fd);
	}
	/* We survived for given time - test succeeded */
	tst_resm(TPASS, "kernel survived inotify beating");

	/* Kill the child creating / deleting files and wait for it */
	kill(pid, SIGKILL);
	wait(NULL);
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++)
		verify_inotify();

	cleanup();
	tst_exit();
}

#else

int main(void)
{
	tst_brkm(TCONF, NULL, "system doesn't have required inotify support");
}

#endif
