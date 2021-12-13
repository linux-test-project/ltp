// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * MODIFIED: - mridge@us.ibm.com -- changed getpid to syscall(get thread ID)
 * for unique ID on NPTL threading
 */

/*\
 * [Description]
 *
 * Check that file locks are removed when a file descriptor is closed, three
 * different tests are implemented.
 *
 * Parent opens a file and duplicates the file descriptor, places locks using
 * both file descriptors then closes one descriptor, all locks should be
 * removed.
 *
 * Open same file twice using open, place locks using both descriptors then
 * close one descriptor, all lock should be removed.
 *
 * Open file twice, each in a different process, set the locks and the child
 * check the locks. Close the first file descriptor and have child check locks
 * again. Only locks set on first file descriptor should have been removed.
 */

#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#define DATA    "ABCDEFGHIJ"
#define DUP     0
#define OPEN    1
#define FORK_   2

static char tmpname[10] = "testfile";
static int fd[2];

static const struct flock lock_one = {
	.l_type = F_WRLCK,
	.l_whence = 0,
	.l_start = 0L,
	.l_len = 5L,
};

static const struct flock lock_two = {
	.l_type = F_WRLCK,
	.l_whence = 0,
	.l_start = 5L,
	.l_len = 5L,
};

static struct tcase {
	int dup_flag;
	int test_num;
	char *dup_flag_name;
} tcases[] = {
	{DUP, 1, "dup"},
	{OPEN, 2, "open"},
	{FORK_, 3, "fork"}
};

static void lock_region_two(int file_flag, int file_mode)
{
	int fd;

	fd = SAFE_OPEN(tmpname, file_flag, file_mode);

	SAFE_FCNTL(fd, F_SETLK, &lock_two);

	TST_CHECKPOINT_WAKE_AND_WAIT(1);

	SAFE_CLOSE(fd);
}

static void do_test(int file_flag, int file_mode, int dup_flag)
{
	int ret, fd;

	fd = SAFE_OPEN(tmpname, file_flag, file_mode);

	if (!fcntl(fd, F_SETLK, &lock_one))
		tst_res(TFAIL, "Succeeded to lock already locked region one");
	else
		tst_res(TPASS, "Failed to lock already locked region one");

	if (!fcntl(fd, F_SETLK, &lock_two))
		tst_res(TFAIL, "Succeeded to lock already locked region two");
	else
		tst_res(TPASS, "Failed to lock already locked region two");

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	if (fcntl(fd, F_SETLK, &lock_one))
		tst_res(TFAIL | TERRNO, "Failed to lock now unlocked region one");
	else
		tst_res(TPASS, "Succeeded to lock now ulocked region two");

	ret = fcntl(fd, F_SETLK, &lock_two);

	if (dup_flag == FORK_) {
		if (ret)
			tst_res(TPASS, "Failed to lock already locked region two");
		else
			tst_res(TFAIL, "Succeeded to lock already locked region two");
	} else {
		if (ret)
			tst_res(TFAIL, "Failed to lock now ulocked region two");
		else
			tst_res(TPASS, "Succeeded to lock now ulocked region two");
	}

	SAFE_CLOSE(fd);
	TST_CHECKPOINT_WAKE(0);
}

static int run_test(int file_flag, int file_mode, int dup_flag)
{
	fd[0] = SAFE_OPEN(tmpname, file_flag, file_mode);
	SAFE_WRITE(1, fd[0], DATA, 10);

	switch (dup_flag) {
	case FORK_:
		if (!SAFE_FORK()) {
			lock_region_two(file_flag, file_mode);
			exit(0);
		}
	break;
	case OPEN:
		fd[1] = SAFE_OPEN(tmpname, file_flag, file_mode);
	break;
	case DUP:
		fd[1] = SAFE_FCNTL(fd[0], F_DUPFD, 0);
	break;
	}

	SAFE_FCNTL(fd[0], F_SETLK, &lock_one);

	// Lock region two or wait until the child locked it
	if (dup_flag != FORK_)
		SAFE_FCNTL(fd[1], F_SETLK, &lock_two);
	else
		TST_CHECKPOINT_WAIT(1);

	if (!SAFE_FORK()) {
		do_test(file_flag, file_mode, dup_flag);
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);

	tst_res(TINFO, "Closing a file descriptor in parent");

	SAFE_CLOSE(fd[0]);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	if (dup_flag != FORK_)
		SAFE_CLOSE(fd[1]);
	else
		TST_CHECKPOINT_WAKE(1);

	SAFE_UNLINK(tmpname);
	return 0;
}

static void verify_fcntl(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "Running test with %s", tc->dup_flag_name);

	run_test(O_CREAT | O_RDWR | O_TRUNC, 0777, tc->dup_flag);
}

static void cleanup(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(fd); i++) {
		if (fd[i] > 0)
			SAFE_CLOSE(fd[i]);
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.forks_child = 1,
	.test = verify_fcntl,
	.needs_checkpoints = 1,
	.cleanup = cleanup,
};
