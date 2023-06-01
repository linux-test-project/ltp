// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  03/2001 - Written by Wayne Boyer
 *
 * Copyright (c) 2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 */

/*\
 * [Description]
 *
 * Test for ENOENT, EEXIST, EINVAL, EACCES, EPERM errors.
 *
 * - ENOENT - No segment exists for the given key and IPC_CREAT was not specified.
 * - EEXIST - the segment exists and IPC_CREAT | IPC_EXCL is given.
 * - EINVAL - A new segment was to be created and size is less than SHMMIN or
 *   greater than SHMMAX. Or a segment for the given key exists, but size is
 *   gran eater than the size of that segment.
 * - EACCES - The user does not have permission to access the shared memory segment.
 * - EPERM - The SHM_HUGETLB flag was specified, but the caller was not
 *   privileged (did not have the CAP_IPC_LOCK capability) and is not a member
 *   of the sysctl_hugetlb_shm_group group.
 * - ENOMEM - The SHM_HUGETLB flag was specified, the caller was privileged but
 *   not have enough hugepage memory space.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/shm.h>
#include <grp.h>
#include "tst_safe_sysv_ipc.h"
#include "tst_kconfig.h"
#include "tst_test.h"
#include "libnewipc.h"
#include "lapi/shm.h"

#define CONFIG_HUGETLBFS "CONFIG_HUGETLBFS"

static int shm_id = -1;
static key_t shmkey, shmkey1;
static struct passwd *pw;

static struct tcase {
	int *shmkey;
	size_t size;
	int flags;
	/*1: nobody expected  0: root expected */
	int exp_user;
	/*1: nobody expected  0: root expected */
	int exp_group;
	int exp_err;
} tcases[] = {
	{&shmkey1, SHM_SIZE, IPC_EXCL, 0, 0, ENOENT},
	{&shmkey, SHM_SIZE, IPC_CREAT | IPC_EXCL, 0, 0, EEXIST},
	{&shmkey1, SHMMIN - 1, IPC_CREAT | IPC_EXCL, 0, 0, EINVAL},
	{&shmkey1, 8192 + 1, IPC_CREAT | IPC_EXCL, 0, 0, EINVAL},
	{&shmkey, SHM_SIZE * 2, IPC_EXCL, 0, 0, EINVAL},
	{&shmkey, SHM_SIZE, SHM_RD, 1, 0, EACCES},
	{&shmkey1, SHM_SIZE, IPC_CREAT | SHM_HUGETLB, 0, 1, EPERM},
	{&shmkey1, SHM_SIZE, IPC_CREAT | SHM_HUGETLB, 0, 0, ENOMEM}
};

static int get_hugetlb_exp_error(void)
{
	long tmp;
	struct tst_kconfig_var kconfig = {
		.id = CONFIG_HUGETLBFS,
		.id_len = sizeof(CONFIG_HUGETLBFS)-1,
	};

	tst_kconfig_read(&kconfig, 1);

	if (kconfig.choice != 'y') {
		tst_res(TINFO, "SHM_HUGETLB not supported by kernel");
		return EINVAL;
	}

	if (FILE_LINES_SCANF("/proc/meminfo", "Hugepagesize: %ld", &tmp)) {
		tst_res(TINFO, "Huge pages not supported by hardware");
		return ENOENT;
	}

	return 0;
}

static void do_test(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	pid_t pid;

	if (tc->exp_user == 0 && tc->exp_group == 0) {
		TST_EXP_FAIL2(shmget(*tc->shmkey, tc->size, tc->flags), tc->exp_err,
			"shmget(%i, %lu, %i)", *tc->shmkey, tc->size, tc->flags);
		return;
	}

	pid = SAFE_FORK();
	if (pid == 0) {
		if (tc->exp_group) {
			SAFE_SETGROUPS(0, NULL);
			SAFE_SETGID(pw->pw_gid);
		}
		SAFE_SETUID(pw->pw_uid);
		TST_EXP_FAIL2(shmget(*tc->shmkey, tc->size, tc->flags), tc->exp_err,
			"shmget(%i, %lu, %i)", *tc->shmkey, tc->size, tc->flags);
		exit(0);
	}
	tst_reap_children();
}

static void setup(void)
{
	struct rlimit rl = { 0, 0 };
	int hugetlb_errno;
	unsigned int i;

	shmkey = GETIPCKEY();
	shmkey1 = GETIPCKEY();

	SAFE_SETRLIMIT(RLIMIT_MEMLOCK, &rl);
	shm_id = SAFE_SHMGET(shmkey, SHM_SIZE, IPC_CREAT | IPC_EXCL);
	pw = SAFE_GETPWNAM("nobody");
	hugetlb_errno = get_hugetlb_exp_error();

	if (!hugetlb_errno)
		return;

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (tcases[i].flags & SHM_HUGETLB)
			tcases[i].exp_err = hugetlb_errno;
	}
}

static void cleanup(void)
{
	if (shm_id >= 0)
		SAFE_SHMCTL(shm_id, IPC_RMID, NULL);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = do_test,
	.tcnt = ARRAY_SIZE(tcases),
	.hugepages = {TST_NO_HUGEPAGES},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/kernel/shmmax", "8192", TST_SR_TCONF_MISSING | TST_SR_TBROK_RO},
		{}
	},
};
