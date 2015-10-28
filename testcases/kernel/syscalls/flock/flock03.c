/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * This test verifies that flock cannot unlock a file locked
 * by another task
 *
 * Test Steps:
 *
 *  Fork a child processes The parent flocks a file with LOCK_EX Child waits
 *  for that to happen, then checks to make sure it is locked.  Child then
 *  tries to unlock the file. If the unlock succeeds, the child attempts to
 *  lock the file with LOCK_EX. The test passes if the child is able to lock
 *  the file.
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include "test.h"

#define FILE_NAME "flock03"

static void setup(void);
static void cleanup(void);
static void childfunc(int);

#ifdef UCLINUX
static int fd_uc;
static void childfunc_uc(void)
{
	childfunc(fd_uc);
}
#endif

char *TCID = "flock03";
int TST_TOTAL = 3;

int main(int argc, char **argv)
{
	int lc;
	pid_t pid;
	int status;
	int fd;

	tst_parse_opts(argc, argv, NULL, NULL);

#ifdef UCLINUX
	maybe_run_child(&childfunc_uc, "ds", &fd_uc, FILE_NAME);
#endif

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		fd = open(FILE_NAME, O_RDWR);

		if (fd == -1)
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "parent failed to open the file");

		pid = FORK_OR_VFORK();

		if (pid == -1)
			tst_brkm(TFAIL | TERRNO, cleanup, "fork() failed");
		if (pid == 0) {
#ifdef UCLINUX
			if (self_exec(argv[0], "ds", fd, FILE_NAME) < 0)
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "self_exec failed");
#else
			childfunc(fd);
#endif
		}

		TEST(flock(fd, LOCK_EX | LOCK_NB));

		if (TEST_RETURN != 0)
			tst_resm(TFAIL | TTERRNO,
				 "Parent: Initial attempt to flock() failed");
		else
			tst_resm(TPASS,
				 "Parent: Initial attempt to flock() passed");

		TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);

		if ((waitpid(pid, &status, 0)) < 0) {
			tst_resm(TFAIL, "wait() failed");
			continue;
		}
		if ((WIFEXITED(status)) && (WEXITSTATUS(status) == 0))
			tst_resm(TPASS, "flock03 Passed");
		else
			tst_resm(TFAIL, "flock03 Failed");

		close(fd);

	}

	cleanup();
	tst_exit();
}

static void childfunc(int fd)
{
	int fd2;

	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);

	fd2 = open(FILE_NAME, O_RDWR);

	if (fd2 == -1) {
		fprintf(stderr, "CHILD: failed to open the file: %s\n",
		        strerror(errno));
		exit(1);
	}

	if (flock(fd2, LOCK_EX | LOCK_NB) != -1) {
		fprintf(stderr, "CHILD: The file was not already locked\n");
		exit(1);
	}

	TEST(flock(fd, LOCK_UN));
	/* XXX: LOCK_UN does not return an error if there was nothing to
	 * unlock.
	 */
	if (TEST_RETURN == -1) {
		fprintf(stderr, "CHILD: Unable to unlock file locked by "
		        "parent: %s\n", strerror(TEST_ERRNO));
		exit(1);
	} else {
		fprintf(stderr, "CHILD: File locked by parent unlocked\n");
	}

	TEST(flock(fd2, LOCK_EX | LOCK_NB));

	if (TEST_RETURN == -1) {
		fprintf(stderr, "CHILD: Unable to lock file after "
		        "unlocking: %s\n", strerror(TEST_ERRNO));
		exit(1);
	} else {
		fprintf(stderr, "CHILD: Locking after unlock passed\n");
	}

	close(fd);
	close(fd2);

	exit(0);
}

static void setup(void)
{
	int fd;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	TST_CHECKPOINT_INIT(tst_rmdir);

	fd = creat(FILE_NAME, 0666);
	if (fd < 0) {
		tst_resm(TBROK, "creating a new file failed");
		cleanup();
	}
	close(fd);
}

static void cleanup(void)
{
	tst_rmdir();
}
