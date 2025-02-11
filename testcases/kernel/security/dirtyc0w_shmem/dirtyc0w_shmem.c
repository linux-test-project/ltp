// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Red Hat, Inc.
 */

/*\
 * This is a regression test for a write race that allowed unprivileged programs
 * to change readonly files located on tmpfs/shmem on the system using
 * userfaultfd "minor fault handling" (CVE-2022-2590).
 */

#include "config.h"

#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pwd.h>

#include "tst_test.h"

#define TMP_DIR "tmp_dirtyc0w_shmem"
#define TEST_FILE TMP_DIR"/testfile"
#define STR "this is not a test\n"

static uid_t nobody_uid;
static gid_t nobody_gid;
static volatile bool child_early_exit;

static void sighandler(int sig)
{
	if (sig == SIGCHLD) {
		child_early_exit = true;
		return;
	}

	_exit(0);
}

static void setup(void)
{
	struct passwd *pw;

	umask(0);

	pw = SAFE_GETPWNAM("nobody");

	nobody_uid = pw->pw_uid;
	nobody_gid = pw->pw_gid;

	SAFE_MKDIR(TMP_DIR, 0664);
	SAFE_MOUNT(TMP_DIR, TMP_DIR, "tmpfs", 0, NULL);
}

static void dirtyc0w_shmem_test(void)
{
	bool failed = false;
	int pid;
	char c;

	SAFE_FILE_PRINTF(TEST_FILE, STR);
	SAFE_CHMOD(TEST_FILE, 0444);

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_SETGID(nobody_gid);
		SAFE_SETUID(nobody_uid);
		SAFE_EXECLP("dirtyc0w_shmem_child", "dirtyc0w_shmem_child", NULL);
	}

	TST_CHECKPOINT_WAIT(0);

	SAFE_SIGNAL(SIGCHLD, sighandler);
	do {
		usleep(100000);

		SAFE_FILE_SCANF(TEST_FILE, "%c", &c);

		if (c != 't') {
			failed = true;
			break;
		}
	} while (tst_remaining_runtime() && !child_early_exit);
	SAFE_SIGNAL(SIGCHLD, SIG_DFL);

	SAFE_KILL(pid, SIGUSR1);
	tst_reap_children();
	SAFE_UNLINK(TEST_FILE);

	if (child_early_exit)
		tst_res(TINFO, "Early child process exit");
	else if (failed)
		tst_res(TFAIL, "Bug reproduced!");
	else
		tst_res(TPASS, "Bug not reproduced");
}

static void cleanup(void)
{
	SAFE_UMOUNT(TMP_DIR);
}

static struct tst_test test = {
	.needs_checkpoints = 1,
	.forks_child = 1,
	.needs_root = 1,
	.runtime = 120,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = dirtyc0w_shmem_test,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "5535be309971"},
		{"CVE", "2022-2590"},
		{}
	}
};
