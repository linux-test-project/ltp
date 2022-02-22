// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

/*\
 * [Description]
 *
 * This test checks if the pidfd_send_signal syscall wrongfully sends
 * a signal to a new process which inherited the PID of the actual
 * target process.
 *
 * In order to do so it is necessary to start a process with a pre-
 * determined PID. This is accomplished by writing to the
 * /proc/sys/kernel/ns_last_pid file.
 *
 * By utilizing this, this test forks two children with the same PID.
 * It is then checked, if the syscall will send a signal to the second
 * child using the pidfd of the first one.
 */

#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "tst_test.h"
#include "lapi/pidfd.h"
#include "tst_safe_pthread.h"

#define PIDTRIES	3

static char *last_pid_file;
static int pidfd, new_pidfd;
static int old_inode, new_inode;

static int get_inode_number(int fd)
{
	struct stat file_stat;

	SAFE_FSTAT(fd, &file_stat);
	return file_stat.st_ino;
}

static void verify_pidfd_send_signal(void)
{
	pid_t pid, new_pid;
	char pid_filename[32];
	char pid_str[16];
	int i, fail;

	fail = 1;
	for (i = 0; i < PIDTRIES; i++) {
		pid = SAFE_FORK();
		if (pid == 0) {
			TST_CHECKPOINT_WAIT(0);
			return;
		}

		sprintf(pid_filename, "/proc/%d", pid);
		pidfd = SAFE_OPEN(pid_filename, O_DIRECTORY | O_CLOEXEC);
		old_inode = get_inode_number(pidfd);

		TST_CHECKPOINT_WAKE(0);
		tst_reap_children();

		/* Manipulate PID for next process */
		sprintf(pid_str, "%d", pid - 1);
		SAFE_FILE_PRINTF(last_pid_file, "%s", pid_str);

		new_pid = SAFE_FORK();
		if (new_pid == 0) {
			TST_CHECKPOINT_WAIT(0);
			return;
		}

		if (new_pid == pid) {
			new_pidfd = SAFE_OPEN(pid_filename,
					O_DIRECTORY | O_CLOEXEC);
			new_inode = get_inode_number(new_pidfd);
			SAFE_CLOSE(new_pidfd);
			fail = 0;
			break;
		}

		if (i < PIDTRIES) {
			tst_res(TINFO,
				"Failed to set correct PID, trying again...");
		}
		SAFE_CLOSE(pidfd);
		TST_CHECKPOINT_WAKE(0);
		tst_reap_children();
	}
	if (fail) {
		tst_brk(TBROK,
			"Could not set new child to same PID as the old one!");
	}
	if (old_inode == new_inode) {
		tst_res(TWARN,
			"File descriptor of new process points to the inode of the old process!");
	}

	TEST(pidfd_send_signal(pidfd, SIGUSR1, NULL, 0));
	if (TST_RET == -1 && TST_ERR == ESRCH) {
		tst_res(TPASS,
			"Did not send signal to wrong process with same PID!");
	} else {
		tst_res(TFAIL | TTERRNO,
			"pidf_send_signal() ended unexpectedly - return value: %ld, error",
			TST_RET);
	}
	TST_CHECKPOINT_WAKE(0);
	tst_reap_children();

	SAFE_CLOSE(pidfd);
}

static void setup(void)
{
	pidfd_send_signal_supported();

	last_pid_file = "/proc/sys/kernel/ns_last_pid";
	if (access(last_pid_file, F_OK) == -1) {
		tst_brk(TCONF, "%s does not exist, cannot set PIDs",
			last_pid_file);
	}
}

static void cleanup(void)
{
	tst_reap_children();
	if (new_pidfd > 0)
		SAFE_CLOSE(new_pidfd);
	if (pidfd > 0)
		SAFE_CLOSE(pidfd);
}

static struct tst_test test = {
	.test_all = verify_pidfd_send_signal,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.forks_child = 1,
};
