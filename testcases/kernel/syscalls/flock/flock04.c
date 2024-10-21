// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2003-2021
 * Author: Vatsal Avasthi
 */

/*\
 * [Description]
 *
 * Test verifies that flock() behavior with different locking combinations along
 * with LOCK_SH and LOCK_EX:
 *
 * - flock() succeeded in acquiring shared lock on shared lock file.
 * - flock() failed to acquire exclusive lock on shared lock file.
 * - flock() failed to acquire shared lock on exclusive lock file.
 * - flock() failed to acquire exclusive lock on exclusive lock file.
 */

#include <errno.h>
#include <sys/file.h>
#include <stdlib.h>

#include "tst_test.h"

static struct tcase {
	int operation;
	char *f_lock;
} tcases[] = {
	{LOCK_SH, "shared lock"},
	{LOCK_EX, "exclusive lock"},
};

static void child(int opt, int should_pass, char *lock)
{
	int retval, fd1;

	fd1 = SAFE_OPEN("testfile", O_RDWR);
	retval = flock(fd1, opt);
	if (should_pass) {
		tst_res(retval == -1 ? TFAIL : TPASS,
			" Child acquiring %s got %d", lock, retval);
	} else {
		tst_res(retval == -1 ? TPASS : TFAIL,
			" Child acquiring %s got %d", lock, retval);
	}

	SAFE_CLOSE(fd1);
	exit(0);
}

static void verify_flock(unsigned n)
{
	int fd2;
	pid_t pid;
	struct tcase *tc = &tcases[n];

	fd2 = SAFE_OPEN("testfile", O_RDWR);
	TEST(flock(fd2, tc->operation));
	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "flock() failed to acquire %s",
			tc->f_lock);
		SAFE_CLOSE(fd2);
		return;
	}

	tst_res(TPASS, "Parent had %s", tc->f_lock);

	pid = SAFE_FORK();
	if (pid == 0)
		child(LOCK_SH | LOCK_NB, tc->operation & LOCK_SH, "shared lock");
	else
		tst_reap_children();

	pid = SAFE_FORK();
	if (pid == 0)
		child(LOCK_EX | LOCK_NB, 0, "exclusive lock");
	else
		tst_reap_children();

	SAFE_CLOSE(fd2);
}

static void setup(void)
{
	int fd;

	fd = SAFE_OPEN("testfile", O_CREAT | O_TRUNC | O_RDWR, 0644);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_flock,
	.needs_tmpdir = 1,
	.setup = setup,
	.forks_child = 1,
};
