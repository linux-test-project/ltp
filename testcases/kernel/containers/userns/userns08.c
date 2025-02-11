// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <rpalethorpe@suse.com>
 */

/*\
 * Reproducer of CVE-2018-18955; broken uid/gid mapping for nested
 * user namespaces with >5 ranges
 *
 * See original reproducer and description by Jan Horn:
 * https://bugs.chromium.org/p/project-zero/issues/detail?id=1712
 *
 * Note that calling seteuid from root can cause the dumpable bit to
 * be unset. The proc files of non dumpable processes are then owned
 * by (the real) root. So on the second level we reset dumpable to 1.
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/mount.h>

#include "tst_test.h"
#include "tst_clone.h"
#include "lapi/sched.h"
#include "tst_safe_file_at.h"

static pid_t clone_newuser(void)
{
	const struct tst_clone_args cargs = {
		.flags = CLONE_NEWUSER,
		.exit_signal = SIGCHLD,
	};

	return SAFE_CLONE(&cargs);
}

static void write_mapping(const pid_t proc_in_ns,
			  const char *const id_mapping)
{
	char proc_path[PATH_MAX];
	int proc_dir;

	sprintf(proc_path, "/proc/%d", (int)proc_in_ns);
	proc_dir = SAFE_OPEN(proc_path, O_DIRECTORY);

	TEST(faccessat(proc_dir, "uid_map", F_OK, 0));
	if (TST_RET && TST_ERR == ENOENT)
		tst_brk(TCONF, "No uid_map file; interface was added in v3.5");

	SAFE_FILE_PRINTFAT(proc_dir, "setgroups", "%s", "deny");
	SAFE_FILE_PRINTFAT(proc_dir, "uid_map", "%s", id_mapping);
	SAFE_FILE_PRINTFAT(proc_dir, "gid_map", "%s", id_mapping);

	SAFE_CLOSE(proc_dir);
}

static void ns_level2(void)
{
	if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0))
		tst_res(TINFO | TERRNO, "Failed to set dumpable flag");
	TST_CHECKPOINT_WAKE_AND_WAIT(1);

	TST_EXP_FAIL(open("restricted", O_WRONLY), EACCES,
		     "Denied write access to ./restricted");

	exit(0);
}

static void ns_level1(void)
{
	const char *const map_over_5 = "0 0 1\n1 1 1\n2 2 1\n3 3 1\n4 4 1\n5 5 990";
	pid_t level2_proc;

	TST_CHECKPOINT_WAIT(0);

	SAFE_SETGID(0);
	SAFE_SETUID(0);

	level2_proc = clone_newuser();
	if (!level2_proc)
		ns_level2();

	TST_CHECKPOINT_WAIT(1);

	write_mapping(level2_proc, map_over_5);

	TST_CHECKPOINT_WAKE(1);
	tst_reap_children();

	exit(0);
}

static void run(void)
{
	pid_t level1_proc;

	SAFE_SETEGID(100000);
	SAFE_SETEUID(100000);

	level1_proc = clone_newuser();
	if (!level1_proc)
		ns_level1();

	SAFE_SETEGID(0);
	SAFE_SETEUID(0);

	write_mapping(level1_proc, "0 100000 1000");

	TST_CHECKPOINT_WAKE(0);
	tst_reap_children();
}

static void setup(void)
{
	int fd = SAFE_OPEN("restricted", O_CREAT | O_WRONLY, 0700);

	SAFE_WRITE(SAFE_WRITE_ALL, fd, "\n", 1);
	SAFE_CLOSE(fd);

	SAFE_TRY_FILE_PRINTF("/proc/sys/user/max_user_namespaces", "%d", 10);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_checkpoints = 1,
	.needs_root = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/user/max_user_namespaces", NULL, TST_SR_SKIP},
		{"/proc/sys/kernel/unprivileged_userns_clone", "1", TST_SR_SKIP},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "d2f007dbe7e4"},
		{"CVE", "CVE-2018-18955"},
		{}
	},
};
