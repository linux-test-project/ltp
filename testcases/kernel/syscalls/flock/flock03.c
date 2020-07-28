// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Author: Cyril Hrubis
 *
 * Test Description:
 *  This test verifies that flock() cannot unlock a file locked by another
 *  task.
 *
 * Test Steps:
 *  Create a new child thread, The parent thread/main process flocks a file
 *  with LOCK_EX child thread wait for that to happen, then checks to make
 *  sure it is locked.  Child thread then tries to unlock the file. If the
 *  unlock succeeds, the child thread attempts to lock the file with LOCK_EX.
 *  The test passes if the child thread is able to lock the file.
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/file.h>
#include <pthread.h>

#include "tst_test.h"

void* childfunc(void* arg)
{
	int fd2;
	int* fdp = (int*) arg;
	int fd = *fdp;

	/* wait for the main process to lock the file */
	TST_CHECKPOINT_WAIT(0);

	/* create a new file descriptor and lock the file */
	fd2 = SAFE_OPEN("testfile", O_RDWR);
	if (flock(fd2, LOCK_EX | LOCK_NB) != -1)
		tst_brk(TBROK, "TEST_FAILED: The file was "
				"not already locked by flock()");

	/* unlock a file locked by main process */
	TEST(flock(fd, LOCK_UN));
	if (TST_RET == -1) {
		tst_brk(TBROK, "TEST_FAILED: Unable to unlock"
				"the file using flock(): %s",
				tst_strerrno(TST_ERR));
		exit(1);
	} else {
		tst_res(TPASS, "PASS:unlocked the locked file using flock()");
	}

	/* Lock the same file after unlocking */
	TEST(flock(fd2, LOCK_EX | LOCK_NB));
	if (TST_RET == -1) {
		tst_brk(TBROK, "TEST_FAILED: Unable to lock "
				"file after unlocking using flock(): %s",
				tst_strerrno(TST_ERR));
		exit(1);
	} else {
		tst_res(TPASS, "PASSED: Locking after unlock using flock() passed");
	}

	SAFE_CLOSE(fd2);

	pthread_exit(0);
}

static void verify_flock(void)
{
	int fd1;
	pthread_t thread;

	fd1 = SAFE_OPEN("testfile", O_RDWR);

	/* Create a thread to test unlock and lock a file
         * which is locked by main process
         */
	if(pthread_create(&thread, NULL, &childfunc, &fd1) != 0)
	{
		tst_brk(TBROK,
			"TEST_FAILED: failed to create thread");
		exit(1);
	}

	TEST(flock(fd1, LOCK_EX | LOCK_NB));
	if (TST_RET != 0) {
		tst_brk(TBROK,
			"TEST_FAILED: Initial attempt to flock() failed");
	} else {
		tst_res(TPASS,
			"PASSED: Initial attempt to flock() passed");
	}

	/* wake up waiting thread and wait for the thread to finish testing*/
	TST_CHECKPOINT_WAKE(0);
	pthread_join(thread, NULL);

	SAFE_CLOSE(fd1);
}

static void setup(void)
{
	int fd;

	/* Create a file to test flock() system call */
	fd = SAFE_OPEN("testfile", O_CREAT | O_TRUNC | O_RDWR, 0666);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_flock,
	.needs_checkpoints = 1,
	.setup = setup,
};
