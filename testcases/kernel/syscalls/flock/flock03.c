// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2012-2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2002-2018
 */

/*\
 * [Description]
 *
 * Verify that flock(2) cannot unlock a file locked by another task.
 *
 * Fork a child processes. The parent flocks a file with LOCK_EX. Child waits
 * for that to happen, then checks to make sure it is locked.  Child then
 * tries to unlock the file. If the unlock succeeds, the child attempts to
 * lock the file with LOCK_EX. The test passes if the child is able to lock
 * the file.
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/file.h>

#include "tst_test.h"

static void childfunc(int fd)
{
	int fd2;
	TST_CHECKPOINT_WAIT(0);

	fd2 = SAFE_OPEN("testfile", O_RDWR);
	if (flock(fd2, LOCK_EX | LOCK_NB) != -1)
		tst_brk(TBROK, "CHILD: The file was not already locked");

	TEST(flock(fd, LOCK_UN));
	if (TST_RET == -1) {
		tst_res(TFAIL, "CHILD: Unable to unlock file locked by "
			"parent: %s", tst_strerrno(TST_ERR));
		exit(1);
	} else {
		tst_res(TPASS, "CHILD: File locked by parent unlocked");
	}

	TEST(flock(fd2, LOCK_EX | LOCK_NB));
	if (TST_RET == -1) {
		tst_res(TFAIL, "CHILD: Unable to unlock file after "
			"unlocking: %s", tst_strerrno(TST_ERR));
		exit(1);
	} else {
		tst_res(TPASS, "Locking after unlock passed");
	}

	SAFE_CLOSE(fd);
	SAFE_CLOSE(fd2);

	exit(0);
}

static void verify_flock(void)
{
	int fd1;
	pid_t pid;

	fd1 = SAFE_OPEN("testfile", O_RDWR);

	pid = SAFE_FORK();
	if (pid == 0)
		childfunc(fd1);

	TEST(flock(fd1, LOCK_EX | LOCK_NB));
	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
			"Parent: Initial attempt to flock() failed");
	} else {
		tst_res(TPASS,
			"Parent: Initial attempt to flock() passed");
	}

	TST_CHECKPOINT_WAKE(0);

	tst_reap_children();

	SAFE_CLOSE(fd1);
}

static void setup(void)
{
	int fd;

	fd = SAFE_OPEN("testfile", O_CREAT | O_TRUNC | O_RDWR, 0666);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_flock,
	.needs_checkpoints = 1,
	.forks_child = 1,
	.setup = setup,
};
